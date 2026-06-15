/**
 * @file    usart.h
 * @brief   USART1 串口驱动 — 函数声明
 *
 * 使用 PA9(TX) + PA10(RX)，支持中断接收和常用格式发送。
 *
 * 硬件连接：
 * @code
 *   STM32 PA9  (TX) ──> USB转TTL RX
 *   STM32 PA10 (RX) ──> USB转TTL TX
 *   STM32 GND       ──> USB转TTL GND (务必共地)
 * @endcode
 *
 * 使用示例：
 * @code
 *   #include "usart.h"
 *   USART1_Init(115200);
 *   USART1_SendString("Hello World!\r\n");
 *   USART1_SendNum(12345);
 * @endcode
 */

#ifndef __USART_H
#define __USART_H

#include "stm32f10x.h"

/**
 * @brief  初始化 USART1
 * @param  baudrate  波特率（9600 / 19200 / 38400 / 57600 / 115200 / 230400 / 460800 / 921600）
 * @note   配置 PA9=复用推挽输出(TX) / PA10=浮空输入(RX)，使能接收中断
 */
void USART1_Init(uint32_t baudrate);

/**
 * @brief  发送单个字节（阻塞）
 * @param  byte  要发送的 8 位数据
 * @note   等待 TXE（发送数据寄存器空）标志后写入 DR
 */
void USART1_SendByte(uint8_t byte);

/**
 * @brief  发送字符串（阻塞）
 * @param  String  以 '\0' 结尾的字符串指针
 * @note   逐字节调用 USART1_SendByte
 */
void USART1_SendString(const char *String);

/**
 * @brief  接收单个字节（阻塞）
 * @return 接收到的 8 位数据
 * @note   等待 RXNE（接收数据寄存器非空）标志后读取 DR
 */
uint8_t USART1_ReceiveByte(void);

/**
 * @brief  发送无符号整数（自动转为十进制 ASCII 字符串）
 * @param  num  要发送的数字
 * @note   内部用取模法提取每位数字，逆序发送
 */
void USART1_SendNum(uint32_t num);

#endif /* __USART_H */
