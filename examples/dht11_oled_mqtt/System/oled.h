/**
 * @file    oled.h
 * @brief   SSD1306 OLED 0.96寸显示屏驱动 — 函数声明
 *
 * 使用软件模拟 I2C 通信（PB8=SCL, PB9=SDA），分辨率 128×64。
 * 字库文件 oled_font.h 提供 8×16 像素 ASCII 字模。
 *
 * 硬件连接：
 * @code
 *   OLED VCC ── 3.3V       // 切勿接 5V，会烧坏！
 *   OLED GND ── GND
 *   OLED SCL ── PB8
 *   OLED SDA ── PB9
 * @endcode
 *
 * 坐标系（8×16 字体）：
 * @code
 *       列(y) 1  2  3 ... 16
 *   行(x) ┌─────────────────
 *    1    │ A  B  C  ...
 *    2    │ ...
 *    3    │ ...
 *    4    │ ...
 * @endcode
 *
 * 使用示例：
 * @code
 *   #include "oled.h"
 *   OLED_Init();
 *   OLED_ShowString(1, 1, "Hello!");
 *   OLED_ShowNum(2, 1, 123, 3);
 * @endcode
 */

#ifndef __OLED_H
#define __OLED_H

#include "stm32f10x.h"

/**
 * @brief  初始化 OLED
 * @note   配置 PB8/PB9 为开漏输出，发送 SSD1306 初始化序列，最后清屏
 */
void OLED_Init(void);

/**
 * @brief  清屏（所有像素熄灭）
 */
void OLED_Clear(void);

/**
 * @brief  在指定位置显示一个 8×16 ASCII 字符
 * @param  Line    行号 (1~4)
 * @param  Column  列号 (1~16)
 * @param  Char    ASCII 字符（如 'A', '1', '!'）
 */
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char);

/**
 * @brief  显示字符串
 * @param  Line    起始行号 (1~4)
 * @param  Column  起始列号 (1~16)
 * @param  String  以 '\0' 结尾的字符串
 * @note   超出右边界不会自动换行
 */
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String);

/**
 * @brief  显示无符号十进制数字（右对齐）
 * @param  Line    行号 (1~4)
 * @param  Column  起始列号 (1~16)
 * @param  Number  要显示的数字
 * @param  Length  显示位数（不足左侧补空格）
 */
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);

/**
 * @brief  显示有符号十进制数字
 * @param  Line    行号 (1~4)
 * @param  Column  起始列号 (1~16)
 * @param  Number  有符号整数
 * @param  Length  数字位数（不含符号位）
 * @note   正数前加 '+'，负数前加 '-'
 */
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length);

/**
 * @brief  显示十六进制数字
 * @param  Line    行号 (1~4)
 * @param  Column  起始列号 (1~16)
 * @param  Number  要显示的数字
 * @param  Length  显示位数
 */
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);

/**
 * @brief  显示二进制数字
 * @param  Line    行号 (1~4)
 * @param  Column  起始列号 (1~16)
 * @param  Number  要显示的数字
 * @param  Length  显示位数
 */
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);

#endif /* __OLED_H */
