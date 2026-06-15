# OLED 0.96寸 显示屏驱动

## 概述

SSD1306 OLED 显示屏驱动，使用软件模拟 I2C 通信。分辨率 128×64，支持字符、数字和中文显示。

## 硬件要求

| 组件 | 型号 | 数量 | 说明 |
|------|------|------|------|
| MCU | STM32F103C8T6 | 1 | |
| OLED | 0.96寸 SSD1306 (I2C) | 1 | 蓝色/黄蓝双色，4 引脚 |
| 上拉电阻 | 10KΩ ×2 | 2 | SCL、SDA 建议上拉 |
| 杜邦线 | 母对母 | 4 | VCC、GND、SCL、SDA |

### 接线

| OLED | STM32 | 说明 |
|------|-------|------|
| VCC | 3.3V | 电源（5V 可能烧坏） |
| GND | GND | 地 |
| SCL | PB8 | I2C 时钟线（软件模拟） |
| SDA | PB9 | I2C 数据线（软件模拟） |

```
OLED VCC ──── 3.3V
OLED GND ──── GND
OLED SCL ──── PB8 ──── 10KΩ ──── 3.3V (可选上拉)
OLED SDA ──── PB9 ──── 10KΩ ──── 3.3V (可选上拉)
```

## 依赖

- `stm32f10x_gpio.h`
- `stm32f10x_rcc.h`
- `oled_font.h`（8×16 字库，含 ASCII 和中文）
- `delay.h`（微秒延时）

## API 说明

### void OLED_Init(void)

初始化 OLED：配置 PB8/PB9 为开漏输出，发送 SSD1306 初始化命令序列。I2C 地址 0x78。

### void OLED_Clear(void)

清屏，所有像素熄灭。

### void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr)

在指定位置显示一个 8×16 的 ASCII 字符。

| 参数 | 类型 | 说明 |
|------|------|------|
| `x` | uint8_t | 行位置 (0~3)，共 4 行 |
| `y` | uint8_t | 列位置 (1~16)，共 16 列 |
| `chr` | uint8_t | ASCII 字符，如 'A'、'1' |

### void OLED_ShowString(uint8_t x, uint8_t y, const char *str)

显示字符串。

| 参数 | 类型 | 说明 |
|------|------|------|
| `x` | uint8_t | 行位置 (0~3) |
| `y` | uint8_t | 列位置 (1~16) |
| `str` | const char* | 以 `\0` 结尾的字符串 |

### void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len)

显示数字（右对齐）。

| 参数 | 类型 | 说明 |
|------|------|------|
| `x` | uint8_t | 行位置 (0~3) |
| `y` | uint8_t | 列位置 (1~16) |
| `num` | uint32_t | 要显示的数字 |
| `len` | uint8_t | 显示位数（不足补空格） |

## 使用示例

```c
#include "oled.h"
#include "delay.h"

int main(void)
{
    OLED_Init();
    OLED_Clear();

    // 第1行: 显示字符串
    OLED_ShowString(1, 1, "Hello STM32!");

    // 第2行: 显示温湿度
    OLED_ShowString(2, 1, "Temp: ");
    OLED_ShowNum(2, 7, 28, 2);
    OLED_ShowChar(2, 9, '.');
    OLED_ShowNum(2, 10, 5, 1);
    OLED_ShowString(2, 11, " C");

    // 第3行: 显示报警
    OLED_ShowString(3, 1, "Status: Normal");

    while (1);
}
```

## 坐标系

```
      列(y) 1  2  3 ... 16
行(x) ┌─────────────────────
  1   │  A   B   C  ...  P
  2   │  Q   R   S  ...  ?
  3   │  ...
  4   │  ...
```

- 行 (x)：0~3，每行 16 像素高（8×16 字体占一行）
- 列 (y)：1~16，每个字符 8 像素宽

## 字库说明

- 文件：`oled_font.h`
- 格式：8×16 像素 / 字符
- 覆盖：ASCII 32~126 + 部分中文
- 字模格式：逐行式（每字节一列 8 像素，16 字节/字符）

## 注意事项

- 务必使用 **3.3V** 供电，5V 会烧坏显示屏
- I2C 地址固定为 0x78（SA0 接地），不可配置
- SCL/SDA 建议外接 10KΩ 上拉电阻到 3.3V
- 软件 I2C 速度较慢，清屏/刷新全屏约需 50ms

## 许可证

MIT License
