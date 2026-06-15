/**
 * @file    delay.c
 * @brief   SysTick 延时函数实现
 *
 * 使用 STM32 内部 SysTick（系统滴答定时器）实现微秒/毫秒/秒级延时。
 * 延时期间阻塞 CPU，不依赖中断。
 *
 * 原理（HCLK = 72MHz）：
 *   SysTick 计数器每 1/72 微秒递减 1。
 *   设置 LOAD = 72 * N，则计数 N 微秒后溢出。
 *   轮询 CTRL 寄存器的 COUNTFLAG 位等待溢出。
 */

#include "stm32f10x.h"  /* SysTick 寄存器定义 */

/**
 * @brief  微秒级延时
 * @param  xus  延时微秒数 (0 ~ 233015)
 *
 * 配置步骤：
 *   1. LOAD  = 72 * xus           —— 重装载值（72MHz 下 1μs = 72 个时钟周期）
 *   2. VAL   = 0                  —— 清空当前计数器
 *   3. CTRL  = 0b0101             —— 选择 HCLK 作为时钟源，使能定时器
 *   4. 等待 CTRL 的 COUNTFLAG 置位 —— 计数到 0 时硬件自动置位
 *   5. CTRL  = 0b0100             —— 关闭定时器（不清空 COUNTFLAG）
 */
void Delay_us(uint32_t xus)
{
    SysTick->LOAD = 72 * xus;               /* 设置重装载值（72 个时钟周期 = 1μs @ 72MHz） */
    SysTick->VAL  = 0x00;                   /* 清空当前计数值 */
    SysTick->CTRL = 0x00000005;             /* [2]=时钟源HCLK [0]=使能定时器 */
    while (!(SysTick->CTRL & 0x00010000));  /* 等待 bit16 COUNTFLAG 置位（计数到0） */
    SysTick->CTRL = 0x00000004;             /* 关闭定时器，保留配置 */
}

/**
 * @brief  毫秒级延时
 * @param  xms  延时毫秒数
 * @note   循环 xms 次，每次调用 Delay_us(1000)
 */
void Delay_ms(uint32_t xms)
{
    while (xms--)
    {
        Delay_us(1000);   /* 1ms = 1000μs */
    }
}

/**
 * @brief  秒级延时
 * @param  xs  延时秒数
 * @note   循环 xs 次，每次调用 Delay_ms(1000)
 */
void Delay_s(uint32_t xs)
{
    while (xs--)
    {
        Delay_ms(1000);   /* 1s = 1000ms */
    }
}
