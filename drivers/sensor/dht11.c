#include "dht11.h"

#define DHT11_OUT_HIGH() GPIO_SetBits(DHT11_PORT, DHT11_PIN)
#define DHT11_OUT_LOW() GPIO_ResetBits(DHT11_PORT, DHT11_PIN)
#define DHT11_IN() (GPIO_ReadInputDataBit(DHT11_PORT, DHT11_PIN))

static void DHT11_Mode_Out_PP(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = DHT11_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DHT11_PORT, &GPIO_InitStructure);
}

static void DHT11_Mode_IPU(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = DHT11_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DHT11_PORT, &GPIO_InitStructure);
}

static void DHT11_DelayUs(uint16_t us) {
    uint32_t i;
    for(i = 0; i < (us * 8); i++) {
        __NOP();
    }
}

static void DHT11_DelayMs(uint16_t ms) {
    uint16_t i;
    for(i = 0; i < ms; i++) {
        DHT11_DelayUs(1000);
    }
}

void DHT11_Init(void) {
    RCC_APB2PeriphClockCmd(DHT11_RCC, ENABLE);
    DHT11_Mode_Out_PP();
    DHT11_OUT_HIGH();
    DHT11_DelayMs(100);
}

uint8_t DHT11_ReadData(DHT11_Data *data) {
    uint8_t buf[5] = {0};
    uint8_t i, j;
    uint32_t timeout;
    
    DHT11_Mode_Out_PP();
    DHT11_OUT_LOW();
    DHT11_DelayMs(18);
    DHT11_OUT_HIGH();
    DHT11_DelayUs(30);
    
    DHT11_Mode_IPU();
    
    timeout = 10000;
    while(DHT11_IN() == 1 && timeout--) {
        DHT11_DelayUs(1);
    }
    if(DHT11_IN() == 1) return 1;
    
    timeout = 10000;
    while(DHT11_IN() == 0 && timeout--) {
        DHT11_DelayUs(1);
    }
    if(DHT11_IN() == 0) return 2;
    
    timeout = 10000;
    while(DHT11_IN() == 1 && timeout--) {
        DHT11_DelayUs(1);
    }
    if(DHT11_IN() == 1) return 3;
    
    for(i = 0; i < 5; i++) {
        buf[i] = 0;
        for(j = 0; j < 8; j++) {
            timeout = 10000;
            while(DHT11_IN() == 0 && timeout--) {
                DHT11_DelayUs(1);
            }
            
            DHT11_DelayUs(40);
            
            buf[i] <<= 1;
            if(DHT11_IN() == 1) {
                buf[i] |= 1;
            }
            
            timeout = 10000;
            while(DHT11_IN() == 1 && timeout--) {
                DHT11_DelayUs(1);
            }
        }
    }
    
    data->humidity = buf[0];
    data->humidity_dec = buf[1];
    data->temperature = buf[2];
    data->temperature_dec = buf[3];
    data->checksum = buf[4];
    
    if((buf[0] + buf[1] + buf[2] + buf[3]) != buf[4]) {
        return 4;
    }
    
    return 0;
}