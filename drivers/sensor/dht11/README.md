# DHT11 温湿度传感器驱动

## 概述

DHT11是一款数字温湿度传感器，采用单总线协议通信。本驱动提供了DHT11的初始化和数据读取功能。

## 特性

- 支持STM32标准库
- 单总线协议实现
- 温度范围：0-50°C
- 湿度范围：20-90%RH

## 硬件连接

| DHT11引脚 | STM32引脚 | 说明 |
|----------|-----------|------|
| VCC | 3.3V/5V | 电源 |
| GND | GND | 地 |
| DATA | PB15 | 数据引脚 |

## API 接口

### DHT11_Init()
初始化DHT11传感器，配置GPIO引脚为输出模式。

### DHT11_ReadData(DHT11_Data *data)
读取温湿度数据。

**参数：**
- `data`: 指向DHT11_Data结构体的指针

**返回值：**
- `0`: 读取成功
- `1`: 响应超时
- `2`: 校验错误

## 使用示例

```c
#include "dht11.h"

DHT11_Data data;
uint8_t ret;

DHT11_Init();

ret = DHT11_ReadData(&data);
if (ret == 0) {
    // 温度整数部分: data.temperature
    // 温度小数部分: data.temperature_dec
    // 湿度整数部分: data.humidity
    // 湿度小数部分: data.humidity_dec
}
```

## 数据结构

```c
typedef struct {
    uint8_t temperature;      // 温度整数部分
    uint8_t temperature_dec;  // 温度小数部分
    uint8_t humidity;         // 湿度整数部分
    uint8_t humidity_dec;     // 湿度小数部分
} DHT11_Data;
```

## 注意事项

1. DATA引脚需要外接10K上拉电阻
2. 读取间隔建议大于1秒
3. DHT11精度为1°C和1%RH