# USART 串口驱动

## 概述

本驱动实现了STM32 USART串口通信功能，支持发送和接收数据。

## 特性

- 支持STM32标准库
- 支持USART1
- 中断接收模式
- 支持字符串和数字发送

## 硬件连接

| USART1引脚 | STM32引脚 | 说明 |
|------------|-----------|------|
| TX | PA9 | 串口发送 |
| RX | PA10 | 串口接收 |

## API 接口

### USART1_Init(uint32_t baudrate)
初始化USART1。

**参数：**
- `baudrate`: 波特率

### USART1_SendByte(uint8_t byte)
发送单个字节。

**参数：**
- `byte`: 要发送的字节

### USART1_SendString(const char *String)
发送字符串。

**参数：**
- `String`: 字符串指针

### USART1_SendNum(uint32_t num)
发送数字。

**参数：**
- `num`: 数字

### USART1_ReceiveByte(void)
接收单个字节（阻塞方式）。

**返回值：**
- 接收到的字节

## 使用示例

```c
#include "usart.h"

USART1_Init(115200);

USART1_SendString("Hello World!\r\n");
USART1_SendNum(12345);
```

## 注意事项

1. USART1使用PA9和PA10引脚
2. 波特率支持常见值：9600, 19200, 38400, 57600, 115200
3. 接收缓冲区大小为256字节