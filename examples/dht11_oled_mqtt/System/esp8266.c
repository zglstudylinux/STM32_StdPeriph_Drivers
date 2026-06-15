/**
 * @file    esp8266.c
 * @brief   ESP8266 WiFi 模块驱动 — AT 指令实现
 *
 * ==================== 工作原理 ====================
 *
 * 本驱动通过 USART2（PA2=TX, PA3=RX）向 ESP8266 发送 AT 指令，
 * 利用中断接收模块返回的应答数据到 rx_buffer。
 *
 * AT 指令流程：
 *   1. ClearBuffer  →  清空接收缓冲区
 *   2. SendString   →  发送 AT 指令 + "\r\n"
 *   3. WaitResponse →  轮询 rx_buffer 等待期望的应答字符串
 *   4. 超时未收到  →  返回 ESP8266_TIMEOUT
 *
 * ==================== 状态机 ====================
 *
 *   DISCONNECTED ──(ConnectWiFi OK)──> WIFI_CONNECTED
 *                                       │
 *                                       ├──(ConnectTCP OK)──> TCP_CONNECTED
 *                                       │                        │
 *                                       ├──(DisconnectTCP)<─────┘
 *                                       │
 *                                       └──(DisconnectWiFi)──> DISCONNECTED
 *
 * ==================== 常用 AT 指令 ====================
 *
 *   AT              →  测试通信        （应返回 OK）
 *   AT+CWMODE=1     →  设为 STA 模式   （客户端模式）
 *   AT+CIPMUX=0     →  单连接模式
 *   AT+CWJAP=...    →  连接 WiFi       （应返回 WIFI GOT IP）
 *   AT+CIPSTART=... →  建立 TCP 连接   （应返回 CONNECT）
 *   AT+CIPSEND=N    →  发送 N 字节     （应返回 > 提示符）
 *   AT+CIPCLOSE     →  关闭 TCP 连接   （应返回 CLOSED）
 *   AT+CWQAP        →  断开 WiFi
 */

#include "esp8266.h"
#include "delay.h"
#include <string.h>
#include <stdio.h>

/* ==================== 模块私有变量 ==================== */

/** @brief 接收缓冲区（中断中填充） */
static char rx_buffer[512];

/** @brief 缓冲区写入索引 */
static uint16_t rx_index = 0;

/** @brief 当前连接状态 */
static ESP8266_ConnState conn_state = ESP8266_DISCONNECTED;

/* ==================== 初始化 ==================== */

/**
 * @brief  初始化 ESP8266（USART2 + NVIC 中断）
 * @param  baudrate  波特率（模块默认 115200）
 *
 * 配置：
 *   PA2 → 复用推挽输出  (USART2_TX → ESP8266_RX)
 *   PA3 → 浮空输入       (USART2_RX → ESP8266_TX)
 *   使能 USART2 接收中断 (RXNE)，中断函数 USART2_IRQHandler
 */
void ESP8266_Init(uint32_t baudrate)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* 使能时钟：GPIOA(APB2) + USART2(APB1) */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    /* PA2 = USART2_TX，复用推挽输出 */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* PA3 = USART2_RX，浮空输入 */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* 配置 USART2：8N1，波特率由参数指定 */
    USART_InitStructure.USART_BaudRate            = baudrate;
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_Parity              = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART2, &USART_InitStructure);

    /* 配置接收中断：USART2 全局中断，抢占优先级 0，子优先级 0 */
    NVIC_InitStructure.NVIC_IRQChannel                   = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* 使能接收中断（RXNE: RX Not Empty） */
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART2, ENABLE);

    ESP8266_ClearBuffer();
    conn_state = ESP8266_DISCONNECTED;
}

/* ==================== 底层收发 ==================== */

/**
 * @brief  通过 USART2 发送一个字节
 */
void ESP8266_SendByte(uint8_t data)
{
    while (!(USART2->SR & USART_FLAG_TXE));  /* 等待发送完成 */
    USART2->DR = data;                        /* 直接写 DR 寄存器 */
}

/**
 * @brief  通过 USART2 发送字符串
 */
void ESP8266_SendString(const char *str)
{
    while (*str)
    {
        ESP8266_SendByte(*str++);
    }
}

/**
 * @brief  清空接收缓冲区
 */
void ESP8266_ClearBuffer(void)
{
    rx_index = 0;
    memset(rx_buffer, 0, sizeof(rx_buffer));
}

/* ==================== 中断服务 ==================== */

/**
 * @brief  USART2 中断服务函数
 * @note   接收到的字节存入 rx_buffer，超出 512 字节则丢弃
 *
 * 需在 stm32f10x_it.c 中声明：void USART2_IRQHandler(void);
 * 或者如果只有一个 ESP8266 使用 USART2，直接在此定义即可。
 */
void USART2_IRQHandler(void)
{
    /* 检查 RXNE 标志：接收数据寄存器非空 */
    if (USART2->SR & USART_FLAG_RXNE)
    {
        uint8_t data = USART2->DR;             /* 读取数据（自动清除 RXNE） */
        if (rx_index < sizeof(rx_buffer) - 1)  /* 留 1 字节给 '\0' */
        {
            rx_buffer[rx_index++] = data;
        }
    }
}

/* ==================== AT 指令核心 ==================== */

/**
 * @brief  等待接收缓冲区中出现指定字符串
 * @param  expected    期望的字符串（如 "OK"、"CONNECT"、"WIFI GOT IP"）
 * @param  timeout_ms  超时时间（毫秒）
 * @return ESP8266_OK      — 收到期望字符串
 *         ESP8266_ERROR   — 收到 ERROR 或 FAIL
 *         ESP8266_TIMEOUT — 超时
 *
 * @note   每 1ms 检查一次，同时检测 ERROR/FAIL 作为失败条件
 */
ESP8266_Status ESP8266_WaitForResponse(const char *expected, uint32_t timeout_ms)
{
    uint32_t start_time = timeout_ms;

    while (start_time > 0)
    {
        /* 检查是否收到期望字符串 */
        if (strstr(rx_buffer, expected) != NULL)
        {
            return ESP8266_OK;
        }

        /* 检查是否收到错误响应 */
        if (strstr(rx_buffer, "ERROR") != NULL ||
            strstr(rx_buffer, "FAIL")  != NULL)
        {
            return ESP8266_ERROR;
        }

        Delay_ms(1);    /* 等待 1ms 再检查 */
        start_time--;
    }

    return ESP8266_TIMEOUT;  /* 超时 */
}

/**
 * @brief  发送 AT 指令并等待应答
 * @param  cmd          AT 指令字符串（如 "AT"、"AT+CWMODE=1"）
 * @param  expected     期望的应答
 * @param  timeout_ms   超时时间（毫秒）
 * @return ESP8266_Status
 *
 * @note   自动追加 "\r\n" 换行符
 */
ESP8266_Status ESP8266_SendCmd(const char *cmd, const char *expected, uint32_t timeout_ms)
{
    ESP8266_ClearBuffer();              /* 先清空旧数据 */
    ESP8266_SendString(cmd);            /* 发送指令 */
    ESP8266_SendString("\r\n");         /* AT 指令以 \r\n 结尾 */
    return ESP8266_WaitForResponse(expected, timeout_ms);
}

/* ==================== WiFi 连接 ==================== */

/**
 * @brief  连接 WiFi 热点
 * @param  ssid      WiFi 名称（2.4GHz）
 * @param  password  WiFi 密码
 * @return ESP8266_OK    — 连接成功（收到 WIFI GOT IP）
 *         ESP8266_ERROR — AT 通信失败或连接被拒
 *
 * 步骤：
 *   1. AT              → 确认模块正常（超时 1s）
 *   2. AT+CWMODE=1     → 设置 Station 模式（超时 2s）
 *   3. AT+CIPMUX=0     → 设置单连接模式（超时 2s）
 *   4. AT+CWJAP=...    → 连接 WiFi（超时 15s，连接较慢）
 *   5. 成功后状态 → WIFI_CONNECTED
 */
ESP8266_Status ESP8266_ConnectWiFi(const char *ssid, const char *password)
{
    ESP8266_Status status;

    /* 1. 测试 AT 通信 */
    status = ESP8266_SendCmd("AT", "OK", 1000);
    if (status != ESP8266_OK) return status;

    /* 2. 设为 Station 模式（客户端） */
    status = ESP8266_SendCmd("AT+CWMODE=1", "OK", 2000);
    if (status != ESP8266_OK) return status;

    /* 3. 设为单连接模式 */
    status = ESP8266_SendCmd("AT+CIPMUX=0", "OK", 2000);
    if (status != ESP8266_OK) return status;

    /* 4. 连接 WiFi（耗时最长，给 15 秒超时） */
    char cmd[128];
    sprintf(cmd, "AT+CWJAP=\"%s\",\"%s\"", ssid, password);
    status = ESP8266_SendCmd(cmd, "WIFI GOT IP", 15000);
    if (status != ESP8266_OK) return status;

    conn_state = ESP8266_WIFI_CONNECTED;  /* 状态 → WiFi 已连接 */
    return ESP8266_OK;
}

/**
 * @brief  断开 WiFi 连接
 */
ESP8266_Status ESP8266_DisconnectWiFi(void)
{
    ESP8266_Status status = ESP8266_SendCmd("AT+CWQAP", "OK", 2000);
    conn_state = ESP8266_DISCONNECTED;
    return status;
}

/**
 * @brief  检查 WiFi 连接状态
 * @note   发送 AT+CIPSTATUS，解析返回的 STATUS:N
 *         STATUS:2 = 已获取 IP（WiFi 已连接）
 *         STATUS:3 = 已建立 TCP 连接
 *         STATUS:4 = TCP 连接断开
 */
ESP8266_Status ESP8266_CheckWiFiConnection(void)
{
    ESP8266_ClearBuffer();
    ESP8266_SendString("AT+CIPSTATUS\r\n");

    ESP8266_Status status = ESP8266_WaitForResponse("STATUS:", 2000);
    if (status == ESP8266_OK)
    {
        /* STATUS:2/3/4 都说明 WiFi 是连着的 */
        if (strstr(rx_buffer, "STATUS:2") ||
            strstr(rx_buffer, "STATUS:3") ||
            strstr(rx_buffer, "STATUS:4"))
        {
            conn_state = ESP8266_WIFI_CONNECTED;
            return ESP8266_OK;
        }
    }
    conn_state = ESP8266_DISCONNECTED;
    return ESP8266_ERROR;
}

/* ==================== TCP 通信 ==================== */

/**
 * @brief  建立 TCP 连接
 * @param  server  服务器 IP 地址（如 "192.168.0.100"）
 * @param  port    端口号（如 1883）
 * @return ESP8266_OK — 连接成功
 *
 * 发送 AT+CIPSTART="TCP","IP",PORT，等待 "CONNECT" 应答。
 * 成功后状态 → TCP_CONNECTED。
 */
ESP8266_Status ESP8266_ConnectTCP(const char *server, uint16_t port)
{
    ESP8266_Status status;
    char cmd[128];

    sprintf(cmd, "AT+CIPSTART=\"TCP\",\"%s\",%d", server, port);
    status = ESP8266_SendCmd(cmd, "CONNECT", 5000);
    if (status != ESP8266_OK) return status;

    conn_state = ESP8266_TCP_CONNECTED;   /* 状态 → TCP 已连接 */
    return ESP8266_OK;
}

/**
 * @brief  通过 TCP 发送数据
 * @param  data  要发送的数据（字符串）
 * @return ESP8266_OK — 发送成功（收到 SEND OK）
 *
 * 流程：
 *   1. AT+CIPSEND=<长度>  →  请求发送
 *   2. 收到 ">" 提示符     →  模块就绪
 *   3. 发送数据            →  直接写入 USART2
 *   4. 等待 "SEND OK"     →  发送成功确认
 */
ESP8266_Status ESP8266_SendTCPData(const char *data)
{
    ESP8266_Status status;
    char cmd[64];
    uint16_t data_len = strlen(data);

    /* 1. 告知模块要发送多少字节 */
    sprintf(cmd, "AT+CIPSEND=%d", data_len);
    status = ESP8266_SendCmd(cmd, ">", 2000);  /* 等待 ">" 提示符 */
    if (status != ESP8266_OK) return status;

    /* 2. 发送实际数据 */
    ESP8266_ClearBuffer();
    ESP8266_SendString(data);

    /* 3. 等待发送确认 */
    status = ESP8266_WaitForResponse("SEND OK", 3000);
    return status;
}

/**
 * @brief  断开 TCP 连接
 * @note   状态回到 WIFI_CONNECTED（WiFi 仍保持连接）
 */
ESP8266_Status ESP8266_DisconnectTCP(void)
{
    ESP8266_Status status = ESP8266_SendCmd("AT+CIPCLOSE", "CLOSED", 2000);
    conn_state = ESP8266_WIFI_CONNECTED;  /* TCP 断开但 WiFi 保持 */
    return status;
}

/* ==================== 辅助函数 ==================== */

/**
 * @brief  获取当前连接状态
 */
ESP8266_ConnState ESP8266_GetState(void)
{
    return conn_state;
}

/**
 * @brief  读取接收缓冲区内容
 * @param  buffer   输出缓冲区
 * @param  max_len  最大读取长度
 * @return 实际读取的字节数，0 表示无数据
 */
uint8_t ESP8266_GetResponse(char *buffer, uint16_t max_len)
{
    if (rx_index > 0 && rx_index < max_len)
    {
        memcpy(buffer, rx_buffer, rx_index);
        return rx_index;
    }
    return 0;
}
