# STM32 外设驱动库

本目录包含可复用的 STM32F103 外设驱动，每个驱动配有独立的 README.md。

## 驱动目录

| 目录 | 驱动 | 功能 | 引脚 |
|------|------|------|------|
| [`system/delay`](system/delay/) | Delay 延时 | SysTick 微秒/毫秒/秒延时 | 无（内部定时器） |
| [`communication/usart`](communication/usart/) | USART 串口 | 串口收发（中断接收） | PA9(TX), PA10(RX) |
| [`communication/esp8266`](communication/esp8266/) | ESP8266 WiFi | AT 指令 WiFi + TCP 通信 | PA2(TX), PA3(RX) |
| [`display/oled`](display/oled/) | OLED 0.96寸 | SSD1306 I2C 显示 | PB8(SCL), PB9(SDA) |
| [`sensor/dht11`](sensor/dht11/) | DHT11 温湿度 | 温度 0~50°C，湿度 20~90% | PB15(DATA) |
| [`control/led_buzzer`](control/led_buzzer/) | LED & 蜂鸣器 | 多路LED+有源蜂鸣器控制 | PC13(LED1), PC14(LED2), PC15(Buzzer) |

## 引脚占用总览

```
      ┌──────────────────────────────┐
      │        STM32F103C8T6         │
      │                              │
  PA2 │ ─── ESP8266 TX (USART2)      │
  PA3 │ ─── ESP8266 RX (USART2)      │
  PA9 │ ─── USART1 TX (串口调试)      │
 PA10 │ ─── USART1 RX (串口调试)      │
  PB8 │ ─── OLED SCL (I2C)           │
  PB9 │ ─── OLED SDA (I2C)           │
 PB15 │ ─── DHT11 DATA               │
 PC13 │ ─── LED1                     │
 PC14 │ ─── LED2                     │
 PC15 │ ─── Buzzer                   │
      │                              │
      └──────────────────────────────┘
```

## 快速使用

1. 将需要的驱动文件（`.c` + `.h`）复制到你的项目 `System/` 目录
2. 在编译配置中添加对应的 `.c` 文件
3. 参考各驱动的 README.md 接线和编码

## 依赖关系

```
delay ─────────────────────────────┐
                                   ↓
usart ──→ esp8266                  │
                                   ↓
dht11 ──→ (主程序) ←── delay ──────┤
                                   ↓
oled ──→ (主程序) ←── delay ───────┤
                                   ↓
led_buzzer ──→ (主程序) ←── delay ─┘
```

所有驱动均依赖 STM32 标准外设库（`stm32f10x_gpio.h`、`stm32f10x_rcc.h` 等）。

## 示例项目

完整的使用示例见 `examples/` 目录：

| 示例 | 使用的驱动 |
|------|-----------|
| [`dht11_oled_mqtt`](../examples/dht11_oled_mqtt/) | delay, usart, dht11, oled, esp8266 |
| [`dht11_oled_mqtt_alarm`](../examples/dht11_oled_mqtt_alarm/) | delay, usart, dht11, oled, esp8266, led_buzzer |

## 许可证

MIT License
