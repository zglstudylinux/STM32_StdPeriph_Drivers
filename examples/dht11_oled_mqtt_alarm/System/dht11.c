/**
 * @file    dht11.c
 * @brief   DHT11 数字温湿度传感器驱动 — 单总线协议实现
 *
 * ==================== 单总线协议（DHT11） ====================
 *
 *   主机（STM32）发起一次读取的流程：
 *
 *   1. 主机拉低 > 18ms  →  起始信号
 *   2. 主机拉高 20~40μs →  释放总线，等待从机响应
 *   3. 从机拉低 80μs    →  响应信号（低电平）
 *   4. 从机拉高 80μs    →  响应信号（高电平）
 *   5. 从机发送 40bit 数据：
 *       每 bit 以 50μs 低电平开始，高电平长度区分 0/1：
 *         - 26~28μs →  bit 0
 *         - 70μs    →  bit 1
 *   6. 数据格式（5 字节）：
 *        Byte0: 湿度整数    (20~90)
 *        Byte1: 湿度小数    (0~9)
 *        Byte2: 温度整数    (0~50)
 *        Byte3: 温度小数    (0~9)
 *        Byte4: 校验和      (Byte0+Byte1+Byte2+Byte3 低 8 位)
 *
 * ==================== GPIO 模式切换 ====================
 *
 *   发送起始信号时  → 推挽输出 (Out_PP)，由 STM32 驱动总线
 *   接收数据时      → 上拉输入 (IPU)，释放总线由传感器驱动
 *
 *   这是单总线协议的核心技巧：同一个 GPIO 引脚在输出和输入之间切换。
 */

#include "dht11.h"

/* ==================== 引脚操作宏 ==================== */

/** @brief 拉高 DATA 引脚 */
#define DHT11_OUT_HIGH()   GPIO_SetBits(DHT11_PORT, DHT11_PIN)

/** @brief 拉低 DATA 引脚 */
#define DHT11_OUT_LOW()    GPIO_ResetBits(DHT11_PORT, DHT11_PIN)

/** @brief 读取 DATA 引脚电平 */
#define DHT11_IN()         GPIO_ReadInputDataBit(DHT11_PORT, DHT11_PIN)

/* ==================== GPIO 模式切换 ==================== */

/**
 * @brief  切换 PA15 为推挽输出模式（主机驱动总线）
 */
static void DHT11_Mode_Out_PP(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;    /* 推挽输出 */
    GPIO_InitStructure.GPIO_Pin   = DHT11_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DHT11_PORT, &GPIO_InitStructure);
}

/**
 * @brief  切换 PA15 为上拉输入模式（释放总线，读取传感器数据）
 */
static void DHT11_Mode_IPU(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;        /* 上拉输入 */
    GPIO_InitStructure.GPIO_Pin   = DHT11_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DHT11_PORT, &GPIO_InitStructure);
}

/* ==================== 本地延时（软件循环） ==================== */

/**
 * @brief  微秒级延时（软件循环，不依赖 SysTick）
 * @param  us  微秒数
 * @note   72MHz 下，8 个 NOP 约等于 1μs（粗略值）
 *         单总线协议对时序要求不严，±20% 误差可接受
 */
static void DHT11_DelayUs(uint16_t us)
{
    uint32_t i;
    for (i = 0; i < (us * 8); i++)
    {
        __NOP();   /* 空操作，约 1/8 μs @ 72MHz */
    }
}

/**
 * @brief  毫秒级延时
 */
static void DHT11_DelayMs(uint16_t ms)
{
    uint16_t i;
    for (i = 0; i < ms; i++)
    {
        DHT11_DelayUs(1000);
    }
}

/* ==================== 初始化 ==================== */

/**
 * @brief  初始化 DHT11
 * @note   使能 GPIOB 时钟，配置 PA15 为推挽输出并拉高（空闲状态）
 *         上电后需等待 ≥1 秒才能进行首次读取
 */
void DHT11_Init(void)
{
    RCC_APB2PeriphClockCmd(DHT11_RCC, ENABLE);  /* 使能 GPIOB 时钟 */
    DHT11_Mode_Out_PP();                         /* 输出模式 */
    DHT11_OUT_HIGH();                            /* 总线空闲 = 高电平 */
    DHT11_DelayMs(100);                          /* 等待传感器稳定 */
}

/* ==================== 数据读取 ==================== */

/**
 * @brief  读取一次温湿度数据（完整单总线协议流程）
 * @param  data  数据存储指针
 * @return 0      — 成功
 *         1      — 从机响应低电平超时
 *         2      — 从机响应高电平超时
 *         3      — 从机释放总线超时
 *         4      — 校验和错误
 *
 * @note   阻塞约 5ms，读取间隔建议 ≥1 秒
 */
uint8_t DHT11_ReadData(DHT11_Data *data)
{
    uint8_t  buf[5] = {0};   /* 5 字节数据缓冲区 */
    uint8_t  i, j;           /* 循环变量 */
    uint32_t timeout;        /* 超时计数器 */

    /* ====== 步骤 1: 主机发送起始信号 ====== */
    DHT11_Mode_Out_PP();     /* 切换到输出模式 */
    DHT11_OUT_LOW();         /* 拉低总线 */
    DHT11_DelayMs(18);       /* 保持低电平 ≥18ms（协议要求） */
    DHT11_OUT_HIGH();        /* 拉高总线 */
    DHT11_DelayUs(30);       /* 保持高电平 20~40μs */

    /* ====== 步骤 2: 切换到输入模式，等待从机响应 ====== */
    DHT11_Mode_IPU();        /* 切换到上拉输入（释放总线） */

    /* --- 2a. 等待从机拉低（响应信号开始） --- */
    timeout = 10000;
    while (DHT11_IN() == 1 && timeout--)   /* 等待总线变低 */
    {
        DHT11_DelayUs(1);
    }
    if (DHT11_IN() == 1) return 1;         /* 超时：从机无响应 */

    /* --- 2b. 等待从机拉高（响应信号结束） --- */
    timeout = 10000;
    while (DHT11_IN() == 0 && timeout--)   /* 等待总线变高 */
    {
        DHT11_DelayUs(1);
    }
    if (DHT11_IN() == 0) return 2;         /* 超时：响应异常 */

    /* --- 2c. 等待从机再次拉低（数据开始） --- */
    timeout = 10000;
    while (DHT11_IN() == 1 && timeout--)
    {
        DHT11_DelayUs(1);
    }
    if (DHT11_IN() == 1) return 3;         /* 超时：从机未释放总线 */

    /* ====== 步骤 3: 读取 40 bit 数据（5 字节） ====== */
    for (i = 0; i < 5; i++)                /* 5 个字节 */
    {
        buf[i] = 0;
        for (j = 0; j < 8; j++)            /* 每个字节 8 位 */
        {
            /* 等待 50μs 低电平结束（bit 起始标志） */
            timeout = 10000;
            while (DHT11_IN() == 0 && timeout--)
            {
                DHT11_DelayUs(1);
            }

            DHT11_DelayUs(40);             /* 延时 40μs 后采样 */

            /* 采样：高电平持续 >28μs → bit 1，否则 bit 0 */
            buf[i] <<= 1;                  /* 左移腾出最低位 */
            if (DHT11_IN() == 1)
            {
                buf[i] |= 1;               /* 高电平 → bit 1 */
            }

            /* 等待当前 bit 的高电平结束 */
            timeout = 10000;
            while (DHT11_IN() == 1 && timeout--)
            {
                DHT11_DelayUs(1);
            }
        }  /* end for j */
    }  /* end for i */

    /* ====== 步骤 4: 填充数据结构 ====== */
    data->humidity       = buf[0];   /* 湿度整数 */
    data->humidity_dec   = buf[1];   /* 湿度小数 */
    data->temperature    = buf[2];   /* 温度整数 */
    data->temperature_dec = buf[3];  /* 温度小数 */
    data->checksum       = buf[4];   /* 校验和 */

    /* ====== 步骤 5: 校验 ====== */
    if ((buf[0] + buf[1] + buf[2] + buf[3]) != buf[4])
    {
        return 4;  /* 校验和不匹配 — 数据可能损坏 */
    }

    return 0;  /* 读取成功 */
}
