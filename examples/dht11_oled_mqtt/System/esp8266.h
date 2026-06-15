#ifndef __ESP8266_H
#define __ESP8266_H

#include "stm32f10x.h"

typedef enum {
    ESP8266_OK = 0,
    ESP8266_ERROR = 1,
    ESP8266_TIMEOUT = 2,
    ESP8266_NO_RESPONSE = 3
} ESP8266_Status;

typedef enum {
    ESP8266_DISCONNECTED = 0,
    ESP8266_WIFI_CONNECTED = 1,
    ESP8266_TCP_CONNECTED = 2
} ESP8266_ConnState;

void ESP8266_Init(uint32_t baudrate);
void ESP8266_SendByte(uint8_t data);
void ESP8266_SendString(const char *str);
void ESP8266_ClearBuffer(void);

ESP8266_Status ESP8266_SendCmd(const char *cmd, const char *expected, uint32_t timeout_ms);
ESP8266_Status ESP8266_WaitForResponse(const char *expected, uint32_t timeout_ms);

ESP8266_Status ESP8266_ConnectWiFi(const char *ssid, const char *password);
ESP8266_Status ESP8266_DisconnectWiFi(void);
ESP8266_Status ESP8266_CheckWiFiConnection(void);

ESP8266_Status ESP8266_ConnectTCP(const char *server, uint16_t port);
ESP8266_Status ESP8266_SendTCPData(const char *data);
ESP8266_Status ESP8266_DisconnectTCP(void);

ESP8266_ConnState ESP8266_GetState(void);
uint8_t ESP8266_GetResponse(char *buffer, uint16_t max_len);

#endif