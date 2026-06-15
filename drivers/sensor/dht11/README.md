# DHT11 温湿度传感器驱动

## 概述

DHT11 数字温湿度传感器驱动，通过单总线协议读取温度和湿度数据。

## 硬件要求

| 组件 | 型号 | 数量 | 说明 |
|------|------|------|------|
| MCU | STM32F103C8T6 | 1 | |
| 温湿度传感器 | DHT11 | 1 | 蓝色/白色外壳 |
| 上拉电阻 | 10KΩ | 1 | DATA 引脚需要外接上拉 |
| 杜邦线 | 母对母 | 3 | VCC、GND、DATA |

### 接线

| DHT11 | STM32 | 说明 |
|-------|-------|------|
| VCC (+) | 3.3V 或 5V | 电源 |
| GND (-) | GND | 地 |
| DATA (out) | PB15 | 数据引脚，**需外接 10KΩ 上拉到 VCC** |

```
DHT11 VCC  ──── 3.3V
DHT11 GND  ──── GND
DHT11 DATA ──── PB15 ──┬── 10KΩ ── 3.3V
                       │
                    (上拉电阻)
```

## 依赖

- `stm32f10x_gpio.h`
- `stm32f10x_rcc.h`
- `delay.h`（微秒延时）

## API 说明

### void DHT11_Init(void)

初始化 DHT11，配置 PB15 为推挽输出并拉高。

### uint8_t DHT11_ReadData(DHT11_Data *data)

读取温湿度数据（阻塞约 5ms）。

| 参数 | 类型 | 说明 |
|------|------|------|
| `data` | DHT11_Data* | 数据存储结构体指针 |

| 返回值 | 说明 |
|--------|------|
| 0 | 读取成功 |
| 1 | 传感器响应超时 |
| 2 | 校验和错误 |

### 数据结构

```c
typedef struct {
    uint8_t temperature;      // 温度整数部分 (0~50)
    uint8_t temperature_dec;  // 温度小数部分 (0~9)
    uint8_t humidity;         // 湿度整数部分 (20~90)
    uint8_t humidity_dec;     // 湿度小数部分 (0~9)
} DHT11_Data;
```

## 使用示例

```c
#include "dht11.h"
#include "delay.h"
#include <stdio.h>

int main(void)
{
    DHT11_Data data;
    uint8_t ret;

    DHT11_Init();
    Delay_ms(1000);  // DHT11 上电后需等待 1 秒

    while (1) {
        ret = DHT11_ReadData(&data);
        if (ret == 0) {
            printf("Temp: %d.%d C, Humi: %d.%d %%\n",
                   data.temperature, data.temperature_dec,
                   data.humidity, data.humidity_dec);
        } else {
            printf("DHT11 Read Error: %d\n", ret);
        }
        Delay_ms(2000);  // 采样间隔 ≥ 1 秒
    }
}
```

## 技术参数

| 参数 | 范围 | 精度 |
|------|------|------|
| 温度 | 0 ~ 50°C | ±2°C |
| 湿度 | 20 ~ 90%RH | ±5%RH |
| 采样周期 | ≥ 1 秒 | — |

## 注意事项

- **必须外接 10KΩ 上拉电阻**（从 DATA 到 VCC），否则通信不稳定
- 上电后等待 1 秒再进行第一次读取
- 读取间隔建议 ≥ 2 秒，频繁读取会导致传感器内部发热影响精度
- 校验和 = 温度整数 + 温度小数 + 湿度整数 + 湿度小数（取低 8 位）

## 许可证

MIT License
