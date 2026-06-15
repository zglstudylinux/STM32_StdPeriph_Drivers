# Delay 延时驱动

## 概述

基于 SysTick 定时器的微秒/毫秒/秒延时函数，适用于 STM32F103 系列（HCLK=72MHz）。

## 硬件要求

无需额外硬件，仅依赖 MCU 内部 SysTick 定时器。

| 要求 | 说明 |
|------|------|
| MCU | STM32F103 系列 |
| 系统时钟 | HCLK = 72MHz（使用 STM32 默认 `SystemInit()` 即可） |
| 依赖 | `stm32f10x.h`、`core_cm3.h`（CMSIS） |

## API 说明

```c
void Delay_us(uint32_t us);   // 微秒延时，最大约 233ms
void Delay_ms(uint32_t ms);   // 毫秒延时
void Delay_s(uint32_t s);     // 秒延时
```

### 参数

| 函数 | 参数 | 范围 | 说明 |
|------|------|------|------|
| `Delay_us` | `us` | 0 ~ 233015 | 微秒数 |
| `Delay_ms` | `ms` | 0 ~ 4294967295 | 毫秒数 |
| `Delay_s` | `s` | 0 ~ 4294967295 | 秒数 |

### 实现原理

- 系统时钟 HCLK = 72MHz → 每个 SysTick 周期 = 1/72 μs
- `Delay_us(n)` 设置 `SysTick->LOAD = 72 * n`，轮询 `COUNTFLAG` 直到计数完成
- `Delay_ms` / `Delay_s` 循环调用 `Delay_us`
- 完成后关闭 SysTick，不产生中断

## 使用示例

```c
#include "delay.h"

int main(void)
{
    SystemInit();   // HCLK = 72MHz

    while (1) {
        GPIO_SetBits(GPIOC, GPIO_Pin_13);   // LED ON
        Delay_ms(500);                       // 等待 500ms
        GPIO_ResetBits(GPIOC, GPIO_Pin_13);  // LED OFF
        Delay_ms(500);
    }
}
```

## 注意事项

- 必须先调用 `SystemInit()` 或将系统时钟配置为 72MHz，否则延时时间不准确
- 延时期间阻塞 CPU，不能同时处理其他任务
- 若系统时钟频率不同，需修改 `Delay_us` 中的 `72 * nus` 计算公式

## 许可证

MIT License
