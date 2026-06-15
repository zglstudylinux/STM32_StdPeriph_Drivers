#include "stm32f10x.h"

void USART1_Init(uint32_t BaudRate)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
    
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = BaudRate;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART1, &USART_InitStructure);
    
    USART_Cmd(USART1, ENABLE);
}

void USART1_SendByte(uint8_t Byte)
{
    while (!(USART1->SR & USART_FLAG_TXE));
    USART_SendData(USART1, Byte);
}

void USART1_SendString(char *String)
{
    while (*String)
    {
        USART1_SendByte(*String++);
    }
}

uint8_t USART1_ReceiveByte(void)
{
    while (!(USART1->SR & USART_FLAG_RXNE));
    return USART_ReceiveData(USART1);
}

void USART1_SendNum(uint32_t Number)
{
    char buf[10] = {0};
    uint8_t i = 0, j;
    if (Number == 0) {
        USART1_SendByte('0');
        return;
    }
    while (Number > 0) {
        buf[i++] = Number % 10 + '0';
        Number /= 10;
    }
    for (j = i; j > 0; j--) {
        USART1_SendByte(buf[j-1]);
    }
}