/**
 * @file    delay.h
 * @brief   SysTick 延时函数声明
 * @note    依赖 STM32 标准外设库，系统时钟 HCLK = 72MHz
 *
 * 使用示例：
 * @code
 *   #include "delay.h"
 *   Delay_ms(500);   // 延时 500 毫秒
 *   Delay_us(10);    // 延时 10 微秒
 *   Delay_s(2);      // 延时 2 秒
 * @endcode
 */

#ifndef __DELAY_H
#define __DELAY_H

#include "stm32f10x.h"  /* uint32_t 类型定义 */

/**
 * @brief  微秒级延时（阻塞）
 * @param  us  延时微秒数，最大约 233015（SysTick 24位计数器限制）
 * @note   基于 SysTick 定时器，72MHz 时钟下每周期 = 1/72 μs
 */
void Delay_us(uint32_t us);

/**
 * @brief  毫秒级延时（阻塞）
 * @param  ms  延时毫秒数
 * @note   循环调用 Delay_us(1000) 实现
 */
void Delay_ms(uint32_t ms);

/**
 * @brief  秒级延时（阻塞）
 * @param  s  延时秒数
 * @note   循环调用 Delay_ms(1000) 实现
 */
void Delay_s(uint32_t s);

#endif /* __DELAY_H */
