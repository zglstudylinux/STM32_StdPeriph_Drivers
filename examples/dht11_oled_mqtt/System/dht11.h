/**
 * @file    dht11.h
 * @brief   DHT11 数字温湿度传感器驱动 — 函数与类型声明
 *
 * 单总线（One-Wire）协议通信，使用 PB15 作为数据引脚。
 *
 * 硬件连接：
 * @code
 *   DHT11 VCC  ── 3.3V/5V
 *   DHT11 GND  ── GND
 *   DHT11 DATA ── PB15 ──┬── 10KΩ ── VCC   // 必须外接上拉电阻！
 * @endcode
 *
 * 使用示例：
 * @code
 *   #include "dht11.h"
 *   DHT11_Data data;
 *   DHT11_Init();
 *   Delay_ms(1000);  // 上电后等待 1 秒
 *   if (DHT11_ReadData(&data) == 0) {
 *       printf("Temp: %d.%d C\n", data.temperature, data.temperature_dec);
 *       printf("Humi: %d.%d %%\n", data.humidity, data.humidity_dec);
 *   }
 * @endcode
 */

#ifndef __DHT11_H
#define __DHT11_H

#include "stm32f10x.h"

/* ==================== 引脚宏定义 ==================== */

/** @brief DHT11 数据引脚 — PB15 */
#define DHT11_PIN     GPIO_Pin_15

/** @brief DHT11 所接 GPIO 端口 — GPIOB */
#define DHT11_PORT    GPIOB

/** @brief DHT11 端口时钟 — APB2 的 GPIOB */
#define DHT11_RCC     RCC_APB2Periph_GPIOB

/* ==================== 数据结构 ==================== */

/**
 * @brief  DHT11 读取结果
 * @note   温度范围 0~50°C，湿度范围 20~90%RH
 */
typedef struct {
    uint8_t humidity;         /**< 湿度整数部分 (20~90) */
    uint8_t humidity_dec;     /**< 湿度小数部分 (0~9) */
    uint8_t temperature;      /**< 温度整数部分 (0~50) */
    uint8_t temperature_dec;  /**< 温度小数部分 (0~9) */
    uint8_t checksum;         /**< 校验和（前 4 字节之和低 8 位） */
} DHT11_Data;

/* ==================== 函数声明 ==================== */

void    DHT11_Init(void);
uint8_t DHT11_ReadData(DHT11_Data *data);

#endif /* __DHT11_H */
