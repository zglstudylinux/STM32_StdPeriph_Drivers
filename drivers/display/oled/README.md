# OLED 0.96寸显示屏驱动

## 概述

本驱动支持0.96寸SSD1306 OLED显示屏，采用I2C接口通信。

## 特性

- 支持STM32标准库
- I2C接口通信
- 分辨率：128x64
- 支持字符和数字显示
- 支持中文显示

## 硬件连接

| OLED引脚 | STM32引脚 | 说明 |
|----------|-----------|------|
| VCC | 3.3V | 电源 |
| GND | GND | 地 |
| SCL | PB8 | I2C时钟线 |
| SDA | PB9 | I2C数据线 |

## API 接口

### OLED_Init()
初始化OLED显示屏。

### OLED_Clear()
清屏。

### OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr)
显示单个字符。

**参数：**
- `x`: 列位置 (0-127)
- `y`: 行位置 (0-7)
- `chr`: 字符ASCII码

### OLED_ShowString(uint8_t x, uint8_t y, const char *str)
显示字符串。

**参数：**
- `x`: 列位置 (0-127)
- `y`: 行位置 (0-7)
- `str`: 字符串指针

### OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len)
显示数字。

**参数：**
- `x`: 列位置
- `y`: 行位置
- `num`: 数字
- `len`: 显示位数

## 使用示例

```c
#include "oled.h"

OLED_Init();
OLED_Clear();

OLED_ShowString(0, 0, "Temperature:");
OLED_ShowNum(0, 2, 25, 2);
OLED_ShowChar(0, 4, 'C');
```

## 字符编码

- ASCII字符集
- 支持中文GB2312编码

## 注意事项

1. I2C地址默认为0x78
2. 建议使用3.3V供电
3. SCL和SDA建议外接10K上拉电阻