/**
 * @file    esp8266.h
 * @brief   ESP8266 WiFi 模块驱动 — 函数与类型声明
 *
 * 通过 AT 指令控制 ESP8266，实现 WiFi 连接和 TCP 客户端通信。
 * 使用 USART2（PA2=TX, PA3=RX）与模块通信，中断接收响应数据。
 *
 * 硬件连接：
 * @code
 *   ESP8266 TX  ──> STM32 PA3 (USART2_RX)   // 交叉连接
 *   ESP8266 RX  ──> STM32 PA2 (USART2_TX)
 *   ESP8266 VCC ──> 3.3V                   // 必须 3.3V，不可接 5V
 *   ESP8266 GND ──> GND
 *   ESP8266 CH_PD ──> 3.3V                 // 使能引脚，必须拉高
 * @endcode
 *
 * 使用示例：
 * @code
 *   #include "esp8266.h"
 *   ESP8266_Init(115200);
 *   ESP8266_ConnectWiFi("SSID", "PASSWORD");
 *   ESP8266_ConnectTCP("192.168.0.100", 1883);
 *   ESP8266_SendTCPData("Hello!");
 *   ESP8266_DisconnectTCP();
 * @endcode
 */

#ifndef __ESP8266_H
#define __ESP8266_H

#include "stm32f10x.h"

/* ==================== 状态/返回值枚举 ==================== */

/** @brief AT 指令执行结果 */
typedef enum {
    ESP8266_OK          = 0,  /**< 操作成功 */
    ESP8266_ERROR       = 1,  /**< 通用错误（模块返回 ERROR/FAIL） */
    ESP8266_TIMEOUT     = 2,  /**< 等待响应超时 */
    ESP8266_NO_RESPONSE = 3   /**< 无响应 */
} ESP8266_Status;

/** @brief WiFi/TCP 连接状态 */
typedef enum {
    ESP8266_DISCONNECTED     = 0,  /**< 未连接 WiFi */
    ESP8266_WIFI_CONNECTED   = 1,  /**< WiFi 已连接，TCP 未连接 */
    ESP8266_TCP_CONNECTED    = 2   /**< TCP 已连接 */
} ESP8266_ConnState;

/* ==================== 基础通信函数 ==================== */

void ESP8266_Init(uint32_t baudrate);
void ESP8266_SendByte(uint8_t data);
void ESP8266_SendString(const char *str);
void ESP8266_ClearBuffer(void);

/* ==================== AT 指令封装 ==================== */

ESP8266_Status ESP8266_SendCmd(const char *cmd, const char *expected, uint32_t timeout_ms);
ESP8266_Status ESP8266_WaitForResponse(const char *expected, uint32_t timeout_ms);

/* ==================== WiFi 功能 ==================== */

ESP8266_Status ESP8266_ConnectWiFi(const char *ssid, const char *password);
ESP8266_Status ESP8266_DisconnectWiFi(void);
ESP8266_Status ESP8266_CheckWiFiConnection(void);

/* ==================== TCP 功能 ==================== */

ESP8266_Status ESP8266_ConnectTCP(const char *server, uint16_t port);
ESP8266_Status ESP8266_SendTCPData(const char *data);
ESP8266_Status ESP8266_DisconnectTCP(void);

/* ==================== 辅助函数 ==================== */

ESP8266_ConnState ESP8266_GetState(void);
uint8_t ESP8266_GetResponse(char *buffer, uint16_t max_len);

#endif /* __ESP8266_H */
