/**
 * @file    usart.c
 * @brief   USART1 串口驱动 — 实现
 *
 * 功能：
 *   - 初始化 USART1（PA9=TX, PA10=RX）
 *   - 支持阻塞发送（字节/字符串/数字）
 *   - 支持阻塞接收（单字节）
 *   - 接收中断在 stm32f10x_it.c 中实现
 *
 * 波特率计算（72MHz APB2 时钟）：
 *   BaudRate = 72MHz / (16 * USARTDIV)
 *   标准库自动计算 USARTDIV 整数和小数部分
 */

#include "stm32f10x.h"

/**
 * @brief  初始化 USART1
 * @param  BaudRate  波特率（如 115200）
 *
 * 配置流程：
 *   1. 使能 USART1 和 GPIOA 时钟（均在 APB2）
 *   2. PA9  → 复用推挽输出（TX 必须用 AF_PP）
 *   3. PA10 → 浮空输入（RX 用 IN_FLOATING 即可）
 *   4. USART1 → 8N1 格式（8 数据位/无校验/1 停止位）
 *   5. 使能 USART1 外设
 */
void USART1_Init(uint32_t BaudRate)
{
    /* --- 1. 使能时钟 --- */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;

    /* --- 2. PA9 = TX，复用推挽输出 --- */
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;   /* 复用推挽 — USART TX 必须 */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* --- 3. PA10 = RX，浮空输入 --- */
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* --- 4. 配置 USART1 为 8N1 格式 --- */
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate            = BaudRate;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; /* 无硬件流控 */
    USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;  /* 收发双工 */
    USART_InitStructure.USART_Parity              = USART_Parity_No;                /* 无校验 */
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;               /* 1 停止位 */
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;            /* 8 数据位 */
    USART_Init(USART1, &USART_InitStructure);

    /* --- 5. 使能 USART1 --- */
    USART_Cmd(USART1, ENABLE);
}

/**
 * @brief  发送单个字节
 * @param  Byte  8 位数据
 * @note   轮询 TXE 标志（发送数据寄存器为空），置位后写入 DR
 */
void USART1_SendByte(uint8_t Byte)
{
    while (!(USART1->SR & USART_FLAG_TXE));  /* 等待上一个字节发送完成 */
    USART_SendData(USART1, Byte);            /* 写入数据寄存器 */
}

/**
 * @brief  发送字符串
 * @param  String  以 '\0' 结尾的字符串
 * @note   逐字节发送，遇到 '\0' 停止
 */
void USART1_SendString(char *String)
{
    while (*String)
    {
        USART1_SendByte(*String++);  /* 发送当前字符，指针后移 */
    }
}

/**
 * @brief  接收单个字节（阻塞）
 * @return 接收到的字节
 * @note   轮询 RXNE 标志（接收数据寄存器非空），置位后读取 DR
 */
uint8_t USART1_ReceiveByte(void)
{
    while (!(USART1->SR & USART_FLAG_RXNE));  /* 等待收到数据 */
    return USART_ReceiveData(USART1);         /* 读取数据寄存器 */
}

/**
 * @brief  发送无符号整数（十进制 ASCII）
 * @param  Number  要发送的数字
 *
 * 算法：
 *   1. 取模法提取每位数字，存入 buf（逆序）
 *   2. 逆序遍历 buf，逐位发送
 *   3. 特殊处理 Number == 0
 */
void USART1_SendNum(uint32_t Number)
{
    char buf[10] = {0};   /* 10 位足够存 32 位无符号整数最大值 (4294967295) */
    uint8_t i = 0, j;

    /* 数字 0 直接发送 */
    if (Number == 0) {
        USART1_SendByte('0');
        return;
    }

    /* 取模提取每位数字，存入 buf（逆序: buf[0]=个位, buf[1]=十位...） */
    while (Number > 0) {
        buf[i++] = Number % 10 + '0';  /* 转为 ASCII */
        Number /= 10;
    }

    /* 逆序发送（从高位到低位） */
    for (j = i; j > 0; j--) {
        USART1_SendByte(buf[j - 1]);
    }
}
