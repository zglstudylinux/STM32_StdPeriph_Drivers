/**
 * @file    oled.c
 * @brief   SSD1306 OLED 0.96寸显示屏驱动 — 软件 I2C 实现
 *
 * ==================== 通信协议 ====================
 *
 * 本驱动使用 GPIO 模拟 I2C 时序（软件 I2C），不依赖硬件 I2C 外设。
 *
 *   PB8 → SCL（时钟线）
 *   PB9 → SDA（数据线）
 *   两者均配置为开漏输出（Out_OD），支持 I2C 的"线与"特性。
 *
 * ==================== SSD1306 I2C 数据格式 ====================
 *
 *   每个 I2C 事务 = 起始 + 控制字节 + 数据 + 停止
 *
 *   控制字节：0x78 = 0111 1000
 *     bit7~bit1: 0111 100 = SSD1306 I2C 地址（SA0=0 时为 0x3C<<1 = 0x78）
 *     bit0:      0 = 读写位（0=写）
 *
 *   数据字节格式（SSD1306 规定在 I2C 数据前需加控制字节）：
 *     第1个数据字节：0x00 = 后续是命令
 *                   0x40 = 后续是显示数据（GRAM）
 *
 * ==================== 坐标系（128×64 像素） ====================
 *
 *   横向 128 列（column），纵向 64 行（row）。
 *   纵向分为 8 个"页"（Page 0~7），每页 8 像素高。
 *   8×16 字体占 2 页（16 像素高），共显示 4 行文字。
 *
 *        Page 0  ────  行 1 上半
 *        Page 1  ────  行 1 下半
 *        Page 2  ────  行 2 上半
 *        ...
 *        Page 7  ────  行 4 下半
 *
 * ==================== SSD1306 初始化序列 ====================
 *
 *   参考 SSD1306 数据手册的初始化命令序列：
 *   0xAE → 关闭显示
 *   0xD5 → 设置显示时钟分频/振荡频率
 *   0xA8 → 设置多路复用比（0x3F = 64 路）
 *   0xD3 → 设置显示偏移
 *   0x40 → 设置显示起始行
 *   0xA1 → 段重映射（左右镜像）
 *   0xC8 → COM 扫描方向（上下镜像）
 *   0xDA → 设置 COM 硬件配置
 *   0x81 → 设置对比度
 *   0xD9 → 设置预充电周期
 *   0xDB → 设置 VCOMH 电压
 *   0xA4 → 全屏显示（非全亮）
 *   0xA6 → 正常显示（非反色）
 *   0x8D → 使能电荷泵
 *   0xAF → 开启显示
 */

#include "stm32f10x.h"
#include "oled_font.h"    /* 8×16 字库数据 */
#include "delay.h"

/* ==================== I2C 引脚操作宏 ==================== */

/**
 * @brief  设置 SCL 引脚电平
 * @note   PB8 = 时钟线，开漏输出
 */
#define OLED_W_SCL(x)   GPIO_WriteBit(GPIOB, GPIO_Pin_8, (BitAction)(x))

/**
 * @brief  设置 SDA 引脚电平
 * @note   PB9 = 数据线，开漏输出
 */
#define OLED_W_SDA(x)   GPIO_WriteBit(GPIOB, GPIO_Pin_9, (BitAction)(x))

/* ==================== I2C 底层时序 ==================== */

/**
 * @brief  初始化 I2C 引脚（PB8=SCL, PB9=SDA → 开漏输出）
 * @note   开漏输出 + 外部上拉电阻实现 I2C 的"线与"逻辑
 */
void OLED_I2C_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD;    /* 开漏输出（I2C 标准） */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;             /* PB8 = SCL */
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;             /* PB9 = SDA */
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* 总线空闲状态：SCL 和 SDA 均为高 */
    OLED_W_SCL(1);
    OLED_W_SDA(1);
}

/**
 * @brief  发送 I2C 起始信号
 * @note   SCL 高电平期间，SDA 从高到低的跳变 → START 条件
 */
void OLED_I2C_Start(void)
{
    OLED_W_SDA(1);   /* SDA 先拉高 */
    OLED_W_SCL(1);   /* SCL 拉高 */
    OLED_W_SDA(0);   /* SCL 高时 SDA 下降 → START */
    OLED_W_SCL(0);   /* SCL 拉低，准备传输数据 */
}

/**
 * @brief  发送 I2C 停止信号
 * @note   SCL 高电平期间，SDA 从低到高的跳变 → STOP 条件
 */
void OLED_I2C_Stop(void)
{
    OLED_W_SDA(0);   /* SDA 先拉低 */
    OLED_W_SCL(1);   /* SCL 拉高 */
    OLED_W_SDA(1);   /* SCL 高时 SDA 上升 → STOP */
}

/**
 * @brief  通过 I2C 发送一个字节（MSB 优先）
 * @param  Byte  要发送的 8 位数据
 *
 * 时序：
 *   每个 bit 在 SCL 低电平时改变 SDA，SCL 上升沿被从机采样。
 *   发送完 8 位后，主机释放 SDA 等待从机应答（ACK）。
 */
void OLED_I2C_SendByte(uint8_t Byte)
{
    uint8_t i;

    /* 发送 8 位数据，高位在前（MSB first） */
    for (i = 0; i < 8; i++)
    {
        /* 将当前最高位放到 SDA 上（0x80 >> i 逐位右移） */
        OLED_W_SDA(Byte & (0x80 >> i));

        /* SCL 上升沿：从机采样 SDA */
        OLED_W_SCL(1);
        /* SCL 下降沿：允许改变 SDA */
        OLED_W_SCL(0);
    }

    /* 第 9 个时钟：从机应答（ACK） */
    OLED_W_SCL(1);   /* 主机释放 SCL */
    OLED_W_SCL(0);   /* 准备下一字节 */
}

/* ==================== SSD1306 命令/数据发送 ====================

/**
 * @brief  向 SSD1306 发送一个命令字节
 * @param  Command  命令码（见 SSD1306 数据手册）
 *
 * I2C 格式：START + 0x78 + 0x00 + Command + STOP
 *   0x78 = 设备地址（写）
 *   0x00 = 控制字节：后续是命令（Co=0, D/C#=0）
 */
void OLED_WriteCommand(uint8_t Command)
{
    OLED_I2C_Start();
    OLED_I2C_SendByte(0x78);     /* SSD1306 I2C 地址 (SA0=0, R/W=0) */
    OLED_I2C_SendByte(0x00);     /* 控制字节: 命令模式 */
    OLED_I2C_SendByte(Command);  /* 命令码 */
    OLED_I2C_Stop();
}

/**
 * @brief  向 SSD1306 发送一个数据字节（写入 GDDRAM）
 * @param  Data  像素数据（1 字节 = 纵向 8 像素，1=亮, 0=灭）
 *
 * I2C 格式：START + 0x78 + 0x40 + Data + STOP
 *   0x40 = 控制字节：后续是显示数据（Co=0, D/C#=1）
 */
void OLED_WriteData(uint8_t Data)
{
    OLED_I2C_Start();
    OLED_I2C_SendByte(0x78);     /* SSD1306 I2C 地址 (SA0=0, R/W=0) */
    OLED_I2C_SendByte(0x40);     /* 控制字节: 数据模式 */
    OLED_I2C_SendByte(Data);     /* 像素数据 */
    OLED_I2C_Stop();
}

/**
 * @brief  设置光标位置（GDDRAM 地址指针）
 * @param  Y  页地址 (0~7)，每页 8 像素高
 * @param  X  列地址 (0~127)
 *
 * 页地址命令：0xB0 | Y   → 选择 Page Y
 * 列地址命令：0x10 高4位 + 0x00 低4位 → 设置列 X
 */
void OLED_SetCursor(uint8_t Y, uint8_t X)
{
    OLED_WriteCommand(0xB0 | Y);                       /* 设置页地址 (0xB0~0xB7) */
    OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));       /* 设置列地址高 4 位 */
    OLED_WriteCommand(0x00 | (X & 0x0F));              /* 设置列地址低 4 位 */
}

/* ==================== 显示功能 ==================== */

/**
 * @brief  清屏（所有像素熄灭）
 * @note   遍历 8 页 × 128 列，全部写入 0x00
 */
void OLED_Clear(void)
{
    uint8_t i, j;
    for (j = 0; j < 8; j++)           /* 8 个页 */
    {
        OLED_SetCursor(j, 0);          /* 页 j，列 0 */
        for (i = 0; i < 128; i++)      /* 每页 128 列 */
        {
            OLED_WriteData(0x00);      /* 写入全 0（熄灭） */
        }
    }
}

/**
 * @brief  显示一个 8×16 ASCII 字符
 * @param  Line    行号 (1~4)，对应 Page 0/2/4/6
 * @param  Column  列号 (1~16)，对应 X 坐标 (Column-1)*8
 * @param  Char    ASCII 字符
 *
 * 字符以 8×16 像素显示，占用 2 个页（16 像素高）。
 * 字库索引 = Char - ' ' (空格是第一个可打印字符)。
 *
 * 发送顺序：
 *   1. 页 N 上半：字库前 8 字节（字符上半部分）
 *   2. 页 N+1 下半：字库后 8 字节（字符下半部分）
 */
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char)
{
    uint8_t i;

    /* 映射到实际位置：Line 1→Page 0,1  Line 2→Page 2,3 ... */
    /* 上半部分（页 2*(Line-1)） */
    OLED_SetCursor((Line - 1) * 2, (Column - 1) * 8);
    for (i = 0; i < 8; i++)
    {
        OLED_WriteData(OLED_F8x16[Char - ' '][i]);  /* 前 8 字节 = 上半 */
    }

    /* 下半部分（页 2*(Line-1)+1） */
    OLED_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 8);
    for (i = 0; i < 8; i++)
    {
        OLED_WriteData(OLED_F8x16[Char - ' '][i + 8]);  /* 后 8 字节 = 下半 */
    }
}

/**
 * @brief  显示字符串
 * @param  Line    起始行号 (1~4)
 * @param  Column  起始列号 (1~16)
 * @param  String  以 '\0' 结尾的字符串
 * @note   逐字符调用 OLED_ShowChar，超出右边界不会自动换行
 */
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String)
{
    uint8_t i;
    for (i = 0; String[i] != '\0'; i++)
    {
        OLED_ShowChar(Line, Column + i, String[i]);
    }
}

/**
 * @brief  计算幂（X^Y）
 * @param  X  底数
 * @param  Y  指数
 * @return X 的 Y 次方
 */
uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
    uint32_t Result = 1;
    while (Y--)
    {
        Result *= X;
    }
    return Result;
}

/**
 * @brief  显示无符号十进制数字（右对齐）
 * @param  Line    行号 (1~4)
 * @param  Column  末尾列号，数字向左展开
 * @param  Number  要显示的数字
 * @param  Length  显示位数（不足左侧补空格）
 *
 * 算法：从高位到低位逐位提取数字并显示
 *   Number / 10^(Length-i-1) % 10 → 第 i 位数字
 */
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    uint8_t i;
    for (i = 0; i < Length; i++)
    {
        OLED_ShowChar(Line, Column + i,
            Number / OLED_Pow(10, Length - i - 1) % 10 + '0');
    }
}

/**
 * @brief  显示有符号十进制数字
 * @param  Line    行号
 * @param  Column  起始列号
 * @param  Number  有符号整数
 * @param  Length  数字位数（不含符号）
 */
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length)
{
    uint8_t i;
    uint32_t Number1;

    if (Number >= 0)
    {
        OLED_ShowChar(Line, Column, '+');    /* 正数显示 '+' */
        Number1 = Number;
    }
    else
    {
        OLED_ShowChar(Line, Column, '-');    /* 负数显示 '-' */
        Number1 = -Number;                    /* 取绝对值 */
    }

    /* 数字部分向右偏移 1 列（给符号让位） */
    for (i = 0; i < Length; i++)
    {
        OLED_ShowChar(Line, Column + i + 1,
            Number1 / OLED_Pow(10, Length - i - 1) % 10 + '0');
    }
}

/**
 * @brief  显示十六进制数字
 * @note   数字 0~9 → '0'~'9'，10~15 → 'A'~'F'
 */
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    uint8_t i, SingleNumber;
    for (i = 0; i < Length; i++)
    {
        SingleNumber = Number / OLED_Pow(16, Length - i - 1) % 16;
        if (SingleNumber < 10)
        {
            OLED_ShowChar(Line, Column + i, SingleNumber + '0');
        }
        else
        {
            OLED_ShowChar(Line, Column + i, SingleNumber - 10 + 'A');
        }
    }
}

/**
 * @brief  显示二进制数字
 */
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    uint8_t i;
    for (i = 0; i < Length; i++)
    {
        OLED_ShowChar(Line, Column + i,
            Number / OLED_Pow(2, Length - i - 1) % 2 + '0');
    }
}

/**
 * @brief  初始化 SSD1306 OLED 显示屏
 *
 * 初始化流程：
 *   1. 上电等待 100ms（OLED 内部电源稳定）
 *   2. 初始化 I2C 引脚
 *   3. 发送 SSD1306 初始化命令序列（共 20+ 条命令）
 *   4. 清屏
 *
 * 命令序列说明见文件头部注释。
 */
void OLED_Init(void)
{
    Delay_ms(100);               /* 等待 OLED 上电稳定 */

    OLED_I2C_Init();             /* 初始化 PB8/PB9 为开漏输出 */

    OLED_WriteCommand(0xAE);     /* 关闭显示（配置期间不显示） */

    OLED_WriteCommand(0xD5);     /* 设置显示时钟分频比/振荡频率 */
    OLED_WriteCommand(0x80);     /* 默认值：分频=1, 频率≈370kHz */

    OLED_WriteCommand(0xA8);     /* 设置多路复用比 */
    OLED_WriteCommand(0x3F);     /* 64 路（128×64 分辨率对应 64 COM） */

    OLED_WriteCommand(0xD3);     /* 设置显示偏移 */
    OLED_WriteCommand(0x00);     /* 偏移=0 */

    OLED_WriteCommand(0x40);     /* 设置显示起始行（第 0 行） */

    OLED_WriteCommand(0xA1);     /* 段重映射：column 127 → SEG0（左右镜像） */

    OLED_WriteCommand(0xC8);     /* COM 扫描方向：COM[N-1]→COM0（上下镜像） */

    OLED_WriteCommand(0xDA);     /* 设置 COM 引脚硬件配置 */
    OLED_WriteCommand(0x12);     /* 顺序模式，禁用 COM 左右反转 */

    OLED_WriteCommand(0x81);     /* 设置对比度 */
    OLED_WriteCommand(0xCF);     /* 对比度值 (0~255) */

    OLED_WriteCommand(0xD9);     /* 设置预充电周期 */
    OLED_WriteCommand(0xF1);     /* Phase1=1, Phase2=15 */

    OLED_WriteCommand(0xDB);     /* 设置 VCOMH 电压倍率 */
    OLED_WriteCommand(0x30);     /* ~0.83×VCC */

    OLED_WriteCommand(0xA4);     /* 全屏显示：GDDRAM 内容正常显示 */

    OLED_WriteCommand(0xA6);     /* 正常显示（非反色） */

    OLED_WriteCommand(0x8D);     /* 使能电荷泵 */
    OLED_WriteCommand(0x14);     /* 电荷泵开启（3.3V 下必须开启） */

    OLED_WriteCommand(0xAF);     /* 开启显示 */

    OLED_Clear();                /* 清屏 */
}
