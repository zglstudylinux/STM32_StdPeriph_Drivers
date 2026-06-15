#include "stm32f10x.h"
#include "usart.h"
#include "dht11.h"
#include "oled.h"
#include "esp8266.h"
#include "delay.h"
#include <stdio.h>

#define WIFI_SSID "89878768"
#define WIFI_PASSWORD "89878768"
#define SERVER_IP "192.168.0.85"
#define SERVER_PORT 1883

char send_buffer[128];

int main(void) {
    DHT11_Data data;
    uint8_t ret;
    uint32_t counter = 0;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    USART1_Init(115200);
    USART1_SendString("\r\n=== System Init ===\r\n");
    
    DHT11_Init();
    USART1_SendString("DHT11 Init OK!\r\n");
    
    OLED_Init();
    OLED_Clear();
    OLED_ShowString(1, 1, "Initializing...");
    USART1_SendString("OLED Init OK!\r\n");
    
    ESP8266_Init(115200);
    USART1_SendString("ESP8266 Init OK!\r\n");
    
    ret = ESP8266_ConnectWiFi(WIFI_SSID, WIFI_PASSWORD);
    OLED_Clear();
    if (ret == 0) {
        USART1_SendString("WiFi Connected!\r\n");
        OLED_ShowString(1, 1, "WiFi: Connected");
    } else {
        USART1_SendString("WiFi Connect Failed!\r\n");
        OLED_ShowString(1, 1, "WiFi: Failed");
    }
    
    Delay_ms(1000);
    
    while (1) {
        USART1_SendString("\r\n=== Data #");
        USART1_SendNum(counter);
        USART1_SendString(" ===\r\n");
        
        ret = DHT11_ReadData(&data);
        
        if (ret == 0) {
            USART1_SendString("Humidity: ");
            USART1_SendNum(data.humidity);
            USART1_SendString(".");
            USART1_SendNum(data.humidity_dec);
            USART1_SendString("%\r\n");
            
            USART1_SendString("Temperature: ");
            USART1_SendNum(data.temperature);
            USART1_SendString(".");
            USART1_SendNum(data.temperature_dec);
            USART1_SendString("C\r\n");
            
            OLED_Clear();
            OLED_ShowString(1, 1, "Temp:");
            OLED_ShowNum(1, 6, data.temperature, 2);
            OLED_ShowChar(1, 8, '.');
            OLED_ShowNum(1, 9, data.temperature_dec, 1);
            OLED_ShowString(1, 10, "C");
            
            OLED_ShowString(2, 1, "Humi:");
            OLED_ShowNum(2, 6, data.humidity, 2);
            OLED_ShowChar(2, 8, '.');
            OLED_ShowNum(2, 9, data.humidity_dec, 1);
            OLED_ShowString(2, 10, "%");
            
            sprintf(send_buffer, "{\"temperature\":%d.%d,\"humidity\":%d.%d,\"count\":%d}", 
                    data.temperature, data.temperature_dec, 
                    data.humidity, data.humidity_dec, counter);
            
            ret = ESP8266_ConnectTCP(SERVER_IP, SERVER_PORT);
            if (ret == 0) {
                USART1_SendString("TCP Connected!\r\n");
                OLED_ShowString(3, 1, "TCP: Connected");
                
                ret = ESP8266_SendTCPData(send_buffer);
                if (ret == 0) {
                    USART1_SendString("Data Send OK!\r\n");
                    OLED_ShowString(4, 1, "Send: OK");
                } else {
                    USART1_SendString("Data Send Failed!\r\n");
                    OLED_ShowString(4, 1, "Send: Failed");
                }
                
                ESP8266_DisconnectTCP();
            } else {
                USART1_SendString("TCP Connect Failed!\r\n");
                OLED_ShowString(3, 1, "TCP: Connect Fail");
            }
        } else {
            USART1_SendString("DHT11 Read Error: ");
            USART1_SendNum(ret);
            USART1_SendString("\r\n");
            OLED_Clear();
            OLED_ShowString(1, 1, "DHT11 Error:");
            OLED_ShowNum(1, 12, ret, 1);
        }
        
        counter++;
        Delay_ms(3000);
    }
}