#ifndef __USART_H
#define __USART_H

#include "stm32f10x.h"

void USART1_Init(uint32_t baudrate);
void USART1_SendByte(uint8_t byte);
void USART1_SendString(const char *String);
uint8_t USART1_ReceiveByte(void);
void USART1_SendNum(uint32_t num);

#endif
