/**
 * LED & Buzzer 驱动
 *
 * 硬件连接：
 *   PC13 — LED1（低电平亮）
 *   PC14 — LED2（低电平亮）
 *   PC15 — 有源蜂鸣器（高电平响）
 *
 * 功能：
 *   正常模式：两灯常亮，蜂鸣器不响
 *   报警模式：两灯交替闪烁（1s间隔），蜂鸣器响（由 main.c 调用）
 */

#include "led_buzzer.h"

/**
 * @brief  初始化 LED 和蜂鸣器 GPIO
 * @note   三个设备均在 GPIOC，统一使能时钟并配置
 */
void LED_Buzzer_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* 使能 GPIOC 时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    /* 配置 PC13/PC14/PC15 为推挽输出 */
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin   = LED1_PIN | LED2_PIN | BUZZER_PIN;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

/* ==================== LED1 (PC13) ==================== */

void LED1_On(void)
{
    GPIO_ResetBits(LED1_PORT, LED1_PIN);   /* 低电平点亮 */
}

void LED1_Off(void)
{
    GPIO_SetBits(LED1_PORT, LED1_PIN);     /* 高电平熄灭 */
}

/* ==================== LED2 (PC14) ==================== */

void LED2_On(void)
{
    GPIO_ResetBits(LED2_PORT, LED2_PIN);   /* 低电平点亮 */
}

void LED2_Off(void)
{
    GPIO_SetBits(LED2_PORT, LED2_PIN);     /* 高电平熄灭 */
}

/* ==================== Buzzer (PC15) ==================== */

void Buzzer_On(void)
{
    GPIO_ResetBits(BUZZER_PORT, BUZZER_PIN); /* 低电平触发响 */
}

void Buzzer_Off(void)
{
    GPIO_SetBits(BUZZER_PORT, BUZZER_PIN); /* 高电平静音 */
}

/* ==================== 模式控制 ==================== */

/**
 * @brief  恢复正常模式：两灯常亮，蜂鸣器不响
 */
void Normal_Mode(void)
{
    LED1_On();
    LED2_On();
    Buzzer_Off();
}
