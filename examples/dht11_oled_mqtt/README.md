# DHT11 + OLED + MQTT 温湿度监控项目

## 概述

本项目实现了基于STM32F103C8T6的温湿度监控系统，通过DHT11采集温湿度数据，OLED显示，并通过ESP8266上传到上位机。

## 功能特性

- ✅ DHT11温湿度采集
- ✅ OLED显示温湿度
- ✅ USART串口输出
- ✅ ESP8266 WiFi连接
- ✅ TCP数据上传
- ✅ 计数器功能

## 硬件需求

| 组件 | 型号 | 数量 |
|------|------|------|
| 开发板 | STM32F103C8T6 | 1 |
| 温湿度传感器 | DHT11 | 1 |
| OLED显示屏 | 0.96寸 SSD1306 | 1 |
| WiFi模块 | ESP8266-01 | 1 |
| USB转TTL | CH340/CP2102 | 1 |

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

## 软件环境

- **编译器**: ARM GCC (arm-none-eabi-gcc)
- **构建工具**: Make
- **烧录工具**: OpenOCD + ST-Link
- **上位机**: Python + Tkinter

## 编译说明

### Windows

```bash
cd examples\dht11_oled_mqtt
build.bat
```

### Linux/Mac

```bash
cd examples/dht11_oled_mqtt
make
```

## 烧录说明

```bash
openocd -f interface/stlink.cfg -f stm32f1x_custom.cfg -c "program Project.hex reset exit"
```

## 上位机使用

1. 确保Python已安装
2. 安装依赖：
   ```bash
   pip install tkinter
   ```
3. 运行上位机：
   ```bash
   python tools/mqtt_monitor.py
   ```

## 配置说明

### WiFi配置

修改 `User/main.c` 中的宏定义：

```c
#define WIFI_SSID "你的WiFi名称"
#define WIFI_PASSWORD "你的WiFi密码"
#define SERVER_IP "上位机IP地址"
#define SERVER_PORT 1883
```

## 串口输出格式

```
=== Data #1 ===
Humidity: 76.0%
Temperature: 27.6C
TCP Connected!
Data Send OK!
```

## TCP数据格式

```json
{"temperature":27.6,"humidity":76.0,"count":1}
```

## 项目结构

```
dht11_oled_mqtt/
├── Library/          # STM32标准库
├── Start/            # 启动文件
├── System/           # 驱动代码
│   ├── dht11.c/h
│   ├── oled.c/h
│   ├── esp8266.c/h
│   ├── usart.c/h
│   └── delay.c/h
├── User/             # 用户代码
│   ├── main.c
│   └── stm32f10x_conf.h
├── Makefile
├── build.bat
└── stm32f10x_custom.cfg
```

## 故障排除

### WiFi连接失败
1. 检查WiFi名称和密码是否正确
2. 确保ESP8266模块供电正常
3. 检查串口连接是否正确

### DHT11读取失败
1. 检查DATA引脚连接
2. 确保外接10K上拉电阻
3. 检查传感器是否正常

### TCP连接失败
1. 确保上位机服务器已启动
2. 检查IP地址和端口是否正确
3. 确保设备在同一局域网