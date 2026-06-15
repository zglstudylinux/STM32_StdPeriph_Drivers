# DHT11 + OLED + MQTT + LED闪烁 + 蜂鸣器报警

## 概述

本项目在 `dht11_oled_mqtt` 基础上增加了 **LED 系统指示**和**蜂鸣器温湿度报警**功能。正常运行时两灯常亮、蜂鸣器静音；温湿度超过阈值后 LED 交替闪烁（1秒间隔）+ 蜂鸣器报警，恢复正常后自动解除。

## 功能特性

- ✅ DHT11 温湿度采集
- ✅ OLED 实时显示（含报警状态）
- ✅ USART 串口调试输出（含报警标记）
- ✅ ESP8266 WiFi 连接
- ✅ TCP JSON 数据上传（含报警和 LED 状态字段）
- ✅ **PC13/PC14 LED 双灯系统指示**
- ✅ **PC15 蜂鸣器温湿度超限报警**
- ✅ 上位机 GUI 实时显示报警状态

## 硬件需求

| 组件 | 型号 | 数量 |
|------|------|------|
| 开发板 | STM32F103C8T6 | 1 |
| 温湿度传感器 | DHT11 | 1 |
| OLED显示屏 | 0.96寸 SSD1306 | 1 |
| WiFi模块 | ESP8266-01 | 1 |
| USB转TTL | CH340/CP2102 | 1 |
| 调试器 | ST-Link V2/V3 | 1 |
| **LED** | **3mm/5mm 红色 + 绿色** | **2** |
| **有源蜂鸣器** | **3.3V/5V 低电平触发** | **1** |
| 杜邦线 | 母对母 | 若干 |

## 引脚连接

### DHT11
| DHT11 | STM32 |
|-------|-------|
| VCC | 3.3V |
| GND | GND |
| DATA | PB15 |

### OLED
| OLED | STM32 |
|------|-------|
| VCC | 3.3V |
| GND | GND |
| SCL | PB8 |
| SDA | PB9 |

### ESP8266
| ESP8266 | STM32 |
|---------|-------|
| VCC | 3.3V |
| GND | GND |
| TX | PA3 |
| RX | PA2 |
| CH_PD | 3.3V |

### USB转TTL
| USB转TTL | STM32 |
|----------|-------|
| TX | PA10 |
| RX | PA9 |
| GND | GND |

### LED & 蜂鸣器（🆕 新增）
| 外设 | STM32 | 说明 |
|------|-------|------|
| LED1 | PC13 | 低电平点亮（板载LED可用） |
| LED2 | PC14 | 低电平点亮 |
| Buzzer | PC15 | **低电平触发响**（有源蜂鸣器） |

```
GPIOC:
  PC13 ─── LED1 ─── GND
  PC14 ─── LED2 ─── GND
  PC15 ─── Buzzer ─── GND
```

## 软件环境

| 工具 | 用途 | 下载 |
|------|------|------|
| ARM GCC (14.2+) | 交叉编译器 | [GNU Arm Embedded](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads) |
| OpenOCD | 烧录/调试 | [gnutoolchains.com](https://gnutoolchains.com/arm-eabi/openocd/) |
| Python 3.x | 上位机 | [python.org](https://www.python.org/downloads/) |
| pyserial | 串口通信 | `pip install pyserial` |

### 0. 环境检测（换电脑后第一步）

```bash
bash tools/setup_env.sh   # 自动检测所有工具链，未安装会给出下载链接
```

## 快速开始

### 1. 配置参数

编辑 `User/main.c` 中的宏定义：

```c
#define WIFI_SSID            "你的WiFi名称"
#define WIFI_PASSWORD        "你的WiFi密码"
#define SERVER_IP            "上位机IP地址"
#define SERVER_PORT          1883
#define TEMP_ALARM_THRESHOLD  30     // 温度 >= 30°C 触发报警
#define HUMID_ALARM_THRESHOLD 80     // 湿度 >= 80% 触发报警
```

### 2. 编译 & 烧录

```bash
cd examples/dht11_oled_mqtt_alarm

# 一键编译+烧录
bash build_flash_monitor.sh flash

# 或 Windows CMD
build_and_flash.bat
```

### 3. 启动上位机（⚠️ 必须在 STM32 之前）

```bash
# 先启动 TCP 服务器 GUI
python tools/mqtt_monitor.py &

# 再启动串口监控
python tools/serial_monitor.py COM8
```

> **⚠️ 先启动上位机，再给 STM32 上电/复位！** 否则 TCP 连接会失败。

## 工作模式

| 状态 | LED1 (PC13) | LED2 (PC14) | Buzzer (PC15) | OLED | 触发条件 |
|------|-------------|-------------|---------------|------|----------|
| **正常** | 常亮 | 常亮 | 不响 | 正常显示 | 温度 < 30°C 且 湿度 < 80% |
| **报警** | 交替1s闪烁 | 交替1s闪烁 | 响 | 显示 `** ALARM! **` | 温度 ≥ 30°C 或 湿度 ≥ 80% |

> 报警恢复：当温度**和**湿度都回到阈值以下时，自动恢复正常模式。

## 串口输出

```
=== System Init ===
DHT11 Init OK!
OLED Init OK!
ESP8266 Init OK!
LED & Buzzer Init OK!
WiFi Connected!

=== Data #1 ===
Humidity: 74.0%
Temperature: 27.9C
TCP Connect Failed!

=== Data #10 ===
Humidity: 75.0%
Temperature: 30.2C
*** ALARM! ***        ← 超过阈值
TCP Connected!
Data Send OK!
```

## TCP 数据格式

JSON 包含报警和 LED 状态字段，上位机 GUI 可实时显示：

```json
{
  "temperature": 28.2,
  "humidity": 75.0,
  "count": 7,
  "alarm": 0,
  "led_blink": 0
}
```

| 字段 | 说明 |
|------|------|
| `alarm` | 0=正常, 1=报警中 |
| `led_blink` | 0=常亮, 1=交替闪烁中 |

### 上位机 GUI 显示

启动 `python tools/mqtt_monitor.py` 后：

- 正常：`✓ 正常`（绿色） + `● 常亮`（绿色）
- 报警：`⚠ 报警中!`（红色） + `↯ 闪烁中`（橙色）

## 一键自动化脚本

```bash
cd examples/dht11_oled_mqtt_alarm

bash build_flash_monitor.sh compile   # 仅编译
bash build_flash_monitor.sh flash     # 编译+烧录
bash build_flash_monitor.sh monitor   # 编译+烧录+串口监控
bash build_flash_monitor.sh all       # 完整工作流
bash build_flash_monitor.sh clean     # 清理
```

## 完整操作流程

```
1. [首次] bash tools/setup_env.sh
2. 连接硬件（含 LED ×2 + 蜂鸣器）
3. 修改 User/main.c 的 WiFi、IP、阈值参数
4. bash build_flash_monitor.sh flash
5. 先启动上位机: python tools/mqtt_monitor.py &
6. 复位 STM32（或重新上电）
7. python tools/serial_monitor.py COM8
8. 正常 → 两灯常亮；用手捂热 DHT11 → 超过阈值 → 报警触发
```

## 固件信息

| 项目 | 数值 |
|------|------|
| MCU | STM32F103C8T6 |
| Flash占用 | ~9.4KB / 64KB |
| 编译器 | ARM GCC 14.2.1 |
| 优化等级 | -Os |
| 采样周期 | 3秒（报警期间 LED 每秒翻转） |

## 项目结构

```
dht11_oled_mqtt_alarm/
├── Library/                    # STM32标准库
├── Start/                      # 启动文件
├── System/                     # 驱动代码
│   ├── dht11.c/h              # DHT11温湿度驱动
│   ├── oled.c/h               # OLED显示驱动
│   ├── oled_font.h            # OLED字库
│   ├── esp8266.c/h            # ESP8266 WiFi驱动
│   ├── usart.c/h              # USART串口驱动
│   ├── delay.c/h              # 延时函数
│   └── led_buzzer.c/h         # 🆕 LED + 蜂鸣器驱动
├── User/                       # 用户代码
│   ├── main.c                 # 🆕 含报警逻辑
│   ├── stm32f10x_conf.h
│   └── stm32f10x_it.c/h
├── Makefile / build.bat / ...  # 构建文件
├── build_flash_monitor.sh      # 一键脚本 (推荐)
├── stm32f103c8t6.ld
└── stm32f1x_custom.cfg
```

## 故障排除

### 蜂鸣器一直响 / 一直不响
- 确认蜂鸣器触发方式：本驱动默认**低电平触发**（`Buzzer_On()` = 拉低 PC15）
- 如果你的蜂鸣器是高电平触发，修改 `System/led_buzzer.c` 中的 `Buzzer_On/Off` 电平反转

### 报警不触发
- 确认阈值设置：`>=` 比较整数部分，OLED 显示 `30.x` 即触发（阈值为 30）
- 用手捂热 DHT11 传感器，温度应上升到 30°C 以上

### 其他问题
- 编译/烧录/WiFi/TCP/DHT11/串口 等通用问题参考 `dht11_oled_mqtt/README.md`

## 数据流架构

```
DHT11 ──┬──> OLED (SSD1306 I2C)     实时显示温湿度 + 报警状态
        │
        ├──> USART1 (PA9/PA10)       串口调试输出 (含 *** ALARM! ***)
        │
        ├──> USART2 (PA2/PA3)        ESP8266 WiFi → TCP JSON (含 alarm/led_blink)
        │       └──> 上位机 GUI      实时显示报警 + LED 状态
        │
        └──> 报警判断 (main.c) ──> PC13 LED1  ──┐
                                  PC14 LED2  ──┤ 正常: 双常亮
                                  PC15 Buzzer ──┘ 报警: 交替闪烁 + 蜂鸣
```

## 许可证

MIT License
