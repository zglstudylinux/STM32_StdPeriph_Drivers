#ifndef __DHT11_H
#define __DHT11_H

#include "stm32f10x.h"

#define DHT11_PIN    GPIO_Pin_15
#define DHT11_PORT   GPIOB
#define DHT11_RCC    RCC_APB2Periph_GPIOB

typedef struct {
    uint8_t humidity;
    uint8_t humidity_dec;
    uint8_t temperature;
    uint8_t temperature_dec;
    uint8_t checksum;
} DHT11_Data;

void DHT11_Init(void);
uint8_t DHT11_ReadData(DHT11_Data *data);

#endif