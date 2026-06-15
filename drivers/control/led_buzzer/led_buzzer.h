#ifndef __LED_BUZZER_H
#define __LED_BUZZER_H

#include "stm32f10x.h"

/* 引脚定义 — 全部在 GPIOC */
#define LED1_PORT       GPIOC
#define LED1_PIN        GPIO_Pin_13      /* PC13 — LED1 */

#define LED2_PORT       GPIOC
#define LED2_PIN        GPIO_Pin_14      /* PC14 — LED2 */

#define BUZZER_PORT     GPIOC
#define BUZZER_PIN      GPIO_Pin_15      /* PC15 — 有源蜂鸣器 */

/* 函数声明 */
void LED_Buzzer_Init(void);
void LED1_On(void);
void LED1_Off(void);
void LED2_On(void);
void LED2_Off(void);
void Buzzer_On(void);
void Buzzer_Off(void);
void Normal_Mode(void);

#endif /* __LED_BUZZER_H */
