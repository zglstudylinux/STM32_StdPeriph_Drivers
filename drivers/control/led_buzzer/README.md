# LED & Buzzer 驱动

## 概述

LED 灯和蜂鸣器控制驱动，适用于STM32F103系列。支持多路LED控制和有源蜂鸣器驱动。

## 功能特性

- ✅ 多路 LED 独立控制（开/关）
- ✅ 有源蜂鸣器控制（响/灭）
- ✅ 正常模式：LED 常亮，蜂鸣器静音
- ✅ 可配合 Main 循环实现报警交替闪烁

## 硬件连接

| 外设 | STM32引脚 | 说明 |
|------|-----------|------|
| LED1 | PC13 | 低电平点亮 |
| LED2 | PC14 | 低电平点亮 |
| Buzzer | PC15 | 低电平响（有源蜂鸣器） |

> 所有外设均在 GPIOC，初始化时统一配置。

## API 说明

```c
void LED_Buzzer_Init(void);   // 初始化 GPIOC 时钟及引脚
void LED1_On(void);           // PC13 低电平 → LED1 亮
void LED1_Off(void);          // PC13 高电平 → LED1 灭
void LED2_On(void);           // PC14 低电平 → LED2 亮
void LED2_Off(void);          // PC14 高电平 → LED2 灭
void Buzzer_On(void);         // PC15 低电平 → 蜂鸣器响
void Buzzer_Off(void);        // PC15 高电平 → 蜂鸣器停
void Normal_Mode(void);       // 恢复默认：两灯全亮，蜂鸣器不响
```

## 使用示例

```c
#include "led_buzzer.h"

int main(void)
{
    LED_Buzzer_Init();
    Normal_Mode();  // 默认：两灯常亮，不响

    while (1) {
        if (alarm_condition) {
            Buzzer_On();
            LED1_On();  LED2_Off();
            Delay_ms(1000);
            LED1_Off(); LED2_On();
            Delay_ms(1000);
        } else {
            Normal_Mode();
        }
    }
}
```

## 阈值配置

报警阈值在 `User/main.c` 中通过宏定义配置：

```c
#define TEMP_ALARM_THRESHOLD  28    // 温度 > 28°C 触发报警
#define HUMID_ALARM_THRESHOLD 80    // 湿度 > 80% 触发报警
```

## 许可证

MIT License
