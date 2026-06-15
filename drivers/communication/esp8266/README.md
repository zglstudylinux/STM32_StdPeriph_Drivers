# ESP8266 WiFi模块驱动

## 概述

ESP8266是一款低成本WiFi模块，本驱动通过AT指令控制ESP8266实现WiFi连接和TCP通信。

## 特性

- 支持STM32标准库
- AT指令控制
- WiFi STA模式连接
- TCP客户端通信
- 支持数据发送和接收

## 硬件连接

| ESP8266引脚 | STM32引脚 | 说明 |
|-------------|-----------|------|
| VCC | 3.3V | 电源 |
| GND | GND | 地 |
| TX | PA3 (USART2_RX) | 串口发送 |
| RX | PA2 (USART2_TX) | 串口接收 |
| CH_PD | 3.3V | 使能引脚 |

## API 接口

### ESP8266_Init(uint32_t baudrate)
初始化ESP8266，配置USART通信。

**参数：**
- `baudrate`: 波特率（建议115200）

### ESP8266_ConnectWiFi(const char *ssid, const char *password)
连接WiFi网络。

**参数：**
- `ssid`: WiFi名称
- `password`: WiFi密码

**返回值：**
- `ESP8266_OK`: 连接成功
- `ESP8266_ERROR`: 连接失败
- `ESP8266_TIMEOUT`: 超时

### ESP8266_ConnectTCP(const char *server, uint16_t port)
连接TCP服务器。

**参数：**
- `server`: 服务器IP地址
- `port`: 端口号

**返回值：**
- `ESP8266_OK`: 连接成功
- `ESP8266_ERROR`: 连接失败

### ESP8266_SendTCPData(const char *data)
发送TCP数据。

**参数：**
- `data`: 数据指针

**返回值：**
- `ESP8266_OK`: 发送成功
- `ESP8266_ERROR`: 发送失败

### ESP8266_DisconnectTCP()
断开TCP连接。

## 使用示例

```c
#include "esp8266.h"

ESP8266_Init(115200);

// 连接WiFi
if (ESP8266_ConnectWiFi("SSID", "PASSWORD") == ESP8266_OK) {
    // 连接TCP服务器
    if (ESP8266_ConnectTCP("192.168.0.100", 1883) == ESP8266_OK) {
        ESP8266_SendTCPData("Hello World!");
        ESP8266_DisconnectTCP();
    }
}
```

## 状态枚举

```c
typedef enum {
    ESP8266_DISCONNECTED,
    ESP8266_WIFI_CONNECTED,
    ESP8266_TCP_CONNECTED
} ESP8266_ConnState;
```

## 注意事项

1. ESP8266需要3.3V电源，电流峰值可达200mA
2. TX/RX交叉连接（ESP8266_TX -> STM32_RX）
3. CH_PD引脚必须接高电平使能模块
4. 建议使用硬件流控或降低波特率避免数据丢失