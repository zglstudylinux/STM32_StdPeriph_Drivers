# USART 串口驱动

## 概述

STM32 USART1 串口通信驱动，支持中断接收和常用数据格式发送。使用 PA9(TX) / PA10(RX) 引脚。

## 硬件要求

| 组件 | 型号 | 数量 | 说明 |
|------|------|------|------|
| MCU | STM32F103C8T6 | 1 | 或同系列其他型号 |
| USB转TTL | CH340 / CP2102 / FT232 | 1 | 用于连接电脑查看串口输出 |
| 杜邦线 | 母对母 | 3 | TX、RX、GND |

### 接线

```
STM32 PA9  (TX) ────> USB转TTL RX
STM32 PA10 (RX) ────> USB转TTL TX
STM32 GND       ────> USB转TTL GND
```

| USART1 | STM32引脚 | USB转TTL | 方向 |
|--------|-----------|----------|------|
| TX | PA9 | RX | STM32 → PC |
| RX | PA10 | TX | PC → STM32 |
| GND | GND | GND | 共地 |

## 依赖

- `stm32f10x_usart.h`（标准库）
- `stm32f10x_gpio.h`（标准库）
- `stm32f10x_rcc.h`（标准库）
- `misc.h`（NVIC 配置）

## API 说明

### void USART1_Init(uint32_t baudrate)

初始化 USART1，配置 PA9/PA10 为复用推挽输出，使能接收中断。

| 参数 | 类型 | 说明 |
|------|------|------|
| `baudrate` | uint32_t | 波特率，推荐 115200 |

支持波特率：9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600

### void USART1_SendByte(uint8_t byte)

发送单个字节（阻塞）。

### void USART1_SendString(const char *String)

发送以 `\0` 结尾的字符串（阻塞）。

### void USART1_SendNum(uint32_t num)

发送无符号整数，自动转换为十进制 ASCII。

### uint8_t USART1_ReceiveByte(void)

阻塞接收单个字节。

| 返回值 | 说明 |
|--------|------|
| uint8_t | 接收到的字节 |

## 使用示例

```c
#include "usart.h"

int main(void)
{
    USART1_Init(115200);

    USART1_SendString("=== System Start ===\r\n");
    USART1_SendString("Temperature: ");
    USART1_SendNum(28);
    USART1_SendString(" C\r\n");

    while (1) {
        // 回显收到的数据
        uint8_t ch = USART1_ReceiveByte();
        USART1_SendByte(ch);
    }
}
```

## 连接电脑查看输出

1. USB转TTL 插入电脑
2. 打开串口工具：

```bash
# 使用项目自带工具
python tools/serial_monitor.py COM8 115200

# 或列出所有可用串口
python tools/serial_monitor.py --list
```

## 注意事项

- 接收缓冲区 256 字节，满后新数据丢弃
- 接收使用 USART1 中断（`USART1_IRQHandler`），需在 `stm32f10x_it.c` 中实现
- TX/RX 交叉连接（STM32_TX → USB转TTL_RX）
- 务必共地（GND 互连）

## 许可证

MIT License
