#include "esp8266.h"
#include "delay.h"
#include <string.h>
#include <stdio.h>

static char rx_buffer[512];
static uint16_t rx_index = 0;
static ESP8266_ConnState conn_state = ESP8266_DISCONNECTED;

void ESP8266_Init(uint32_t baudrate) {
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    USART_InitStructure.USART_BaudRate = baudrate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART2, &USART_InitStructure);
    
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART2, ENABLE);
    
    ESP8266_ClearBuffer();
    conn_state = ESP8266_DISCONNECTED;
}

void ESP8266_SendByte(uint8_t data) {
    while (!(USART2->SR & USART_FLAG_TXE));
    USART2->DR = data;
}

void ESP8266_SendString(const char *str) {
    while (*str) {
        ESP8266_SendByte(*str++);
    }
}

void ESP8266_ClearBuffer(void) {
    rx_index = 0;
    memset(rx_buffer, 0, sizeof(rx_buffer));
}

void USART2_IRQHandler(void) {
    if (USART2->SR & USART_FLAG_RXNE) {
        uint8_t data = USART2->DR;
        if (rx_index < sizeof(rx_buffer) - 1) {
            rx_buffer[rx_index++] = data;
        }
    }
}

ESP8266_Status ESP8266_WaitForResponse(const char *expected, uint32_t timeout_ms) {
    uint32_t start_time = timeout_ms;
    
    while (start_time > 0) {
        if (strstr(rx_buffer, expected) != NULL) {
            return ESP8266_OK;
        }
        if (strstr(rx_buffer, "ERROR") != NULL || strstr(rx_buffer, "FAIL") != NULL) {
            return ESP8266_ERROR;
        }
        
        Delay_ms(1);
        start_time--;
    }
    
    return ESP8266_TIMEOUT;
}

ESP8266_Status ESP8266_SendCmd(const char *cmd, const char *expected, uint32_t timeout_ms) {
    ESP8266_ClearBuffer();
    ESP8266_SendString(cmd);
    ESP8266_SendString("\r\n");
    
    return ESP8266_WaitForResponse(expected, timeout_ms);
}

ESP8266_Status ESP8266_ConnectWiFi(const char *ssid, const char *password) {
    ESP8266_Status status;
    
    status = ESP8266_SendCmd("AT", "OK", 1000);
    if (status != ESP8266_OK) return status;
    
    status = ESP8266_SendCmd("AT+CWMODE=1", "OK", 2000);
    if (status != ESP8266_OK) return status;
    
    status = ESP8266_SendCmd("AT+CIPMUX=0", "OK", 2000);
    if (status != ESP8266_OK) return status;
    
    char cmd[128];
    sprintf(cmd, "AT+CWJAP=\"%s\",\"%s\"", ssid, password);
    status = ESP8266_SendCmd(cmd, "WIFI GOT IP", 15000);
    if (status != ESP8266_OK) return status;
    
    conn_state = ESP8266_WIFI_CONNECTED;
    return ESP8266_OK;
}

ESP8266_Status ESP8266_DisconnectWiFi(void) {
    ESP8266_Status status = ESP8266_SendCmd("AT+CWQAP", "OK", 2000);
    conn_state = ESP8266_DISCONNECTED;
    return status;
}

ESP8266_Status ESP8266_CheckWiFiConnection(void) {
    ESP8266_ClearBuffer();
    ESP8266_SendString("AT+CIPSTATUS\r\n");
    
    ESP8266_Status status = ESP8266_WaitForResponse("STATUS:", 2000);
    if (status == ESP8266_OK) {
        if (strstr(rx_buffer, "STATUS:2") || strstr(rx_buffer, "STATUS:3") || strstr(rx_buffer, "STATUS:4")) {
            conn_state = ESP8266_WIFI_CONNECTED;
            return ESP8266_OK;
        }
    }
    conn_state = ESP8266_DISCONNECTED;
    return ESP8266_ERROR;
}

ESP8266_Status ESP8266_ConnectTCP(const char *server, uint16_t port) {
    ESP8266_Status status;
    char cmd[128];
    
    sprintf(cmd, "AT+CIPSTART=\"TCP\",\"%s\",%d", server, port);
    status = ESP8266_SendCmd(cmd, "CONNECT", 5000);
    if (status != ESP8266_OK) return status;
    
    conn_state = ESP8266_TCP_CONNECTED;
    return ESP8266_OK;
}

ESP8266_Status ESP8266_SendTCPData(const char *data) {
    ESP8266_Status status;
    char cmd[64];
    uint16_t data_len = strlen(data);
    
    sprintf(cmd, "AT+CIPSEND=%d", data_len);
    status = ESP8266_SendCmd(cmd, ">", 2000);
    if (status != ESP8266_OK) return status;
    
    ESP8266_ClearBuffer();
    ESP8266_SendString(data);
    
    status = ESP8266_WaitForResponse("SEND OK", 3000);
    return status;
}

ESP8266_Status ESP8266_DisconnectTCP(void) {
    ESP8266_Status status = ESP8266_SendCmd("AT+CIPCLOSE", "CLOSED", 2000);
    conn_state = ESP8266_WIFI_CONNECTED;
    return status;
}

ESP8266_ConnState ESP8266_GetState(void) {
    return conn_state;
}

uint8_t ESP8266_GetResponse(char *buffer, uint16_t max_len) {
    if (rx_index > 0 && rx_index < max_len) {
        memcpy(buffer, rx_buffer, rx_index);
        return rx_index;
    }
    return 0;
}