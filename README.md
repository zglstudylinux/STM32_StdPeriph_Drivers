# STM32_StdPeriph_Drivers

基于STM32标准外设库的嵌入式开发项目仓库，包含各种外设驱动和示例项目。

## 📁 项目结构

```
STM32_StdPeriph_Drivers/
├── README.md                  # 项目说明文档
├── LICENSE                    # 许可证文件
├── .gitignore                 # Git忽略配置
├── docs/                      # 文档目录
├── libraries/                 # STM32标准库（通用）
├── templates/                 # 项目模板
├── drivers/                   # 外设驱动（可复用）
│   ├── communication/         # 通信模块
│   │   ├── esp8266/          # ESP8266 WiFi驱动
│   │   │   ├── esp8266.c/h
│   │   │   └── README.md
│   │   └── usart/            # USART串口驱动
│   │       ├── usart.c/h
│   │       └── README.md
│   ├── display/              # 显示模块
│   │   └── oled/             # OLED驱动
│   │       ├── oled.c/h
│   │       ├── oled_font.h
│   │       └── README.md
│   ├── sensor/               # 传感器模块
│   │   └── dht11/            # DHT11温湿度驱动
│   │       ├── dht11.c/h
│   │       └── README.md
│   └── control/              # 控制模块（预留）
├── examples/                  # 示例项目
│   └── dht11_oled_mqtt/      # 温湿度采集+OLED显示+TCP上传
│       ├── Start/             # 启动文件
│       ├── Library/           # STM32标准库
│       ├── System/            # 系统驱动
│       ├── User/              # 用户代码
│       ├── Makefile           # 编译配置
│       ├── build.bat          # Windows编译脚本
│       ├── stm32f103c8t6.ld   # 链接脚本
│       ├── stm32f1x_custom.cfg # OpenOCD配置
│       └── README.md          # 项目说明
└── tools/                     # 工具脚本
    ├── mqtt_monitor.py        # 上位机监控程序
    └── serial_monitor.py      # 串口监控工具
```

## 🎯 项目目标

- 提供可复用的STM32外设驱动
- 提供完整的示例项目
- 便于其他开发者快速复刻
- 保持代码结构清晰

## 🛠️ 硬件要求

### 核心硬件
- **STM32F103C8T6** 最小系统板
- **ST-Link V2/V3** 调试器

### 外设模块
- **DHT11** 温湿度传感器
- **OLED 0.96寸** SSD1306
- **ESP8266** WiFi模块

## 📦 软件环境配置

### 安装工具链

#### Windows系统
- **ARM GCC工具链**: [Download](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-arm-embedded)
- **OpenOCD**: [Download](https://gnutoolchains.com/arm-eabi/openocd/)
- **MinGW**: [Download](https://sourceforge.net/projects/mingw/)
- **Python 3.x**: [Download](https://www.python.org/downloads/)

### 配置环境变量

将以下路径添加到系统环境变量 `PATH` 中：
```
C:\tools\arm-gcc\bin
C:\tools\openocd\bin
C:\MinGW\bin
```

## 🔧 使用指南

### 克隆仓库

```bash
git clone https://github.com/zglstudylinux/STM32_StdPeriph_Drivers.git
cd STM32_StdPeriph_Drivers
```

### 编译示例项目

```bash
cd examples/dht11_oled_mqtt
build.bat
```

### 烧录程序

```bash
openocd -f interface/stlink.cfg -f stm32f1x_custom.cfg -c "program Project.hex reset exit"
```

## 📚 驱动目录说明

| 目录 | 驱动名称 | 说明 |
|------|----------|------|
| `drivers/communication/esp8266` | ESP8266 WiFi驱动 | 提供WiFi连接和TCP通信功能 |
| `drivers/communication/usart` | USART串口驱动 | 提供串口发送和接收功能 |
| `drivers/display/oled` | OLED显示驱动 | 支持128x64 OLED显示 |
| `drivers/sensor/dht11` | DHT11温湿度驱动 | 读取温湿度数据 |

## 📋 示例项目说明

### dht11_oled_mqtt
**功能**: 温湿度采集 + OLED显示 + WiFi上传
- 使用DHT11采集温湿度数据
- OLED实时显示数据
- ESP8266连接WiFi上传到上位机
- 串口输出调试信息

## 🔍 故障排除

### 编译错误
- **`arm-none-eabi-gcc: command not found`**: 检查ARM GCC工具链安装
- **`make: command not found`**: 安装MinGW并添加到环境变量

### 烧录错误
- **`init mode failed`**: 检查ST-Link连接
- **`UNEXPECTED idcode`**: 使用自定义配置文件

### WiFi连接失败
- 检查WiFi SSID和密码
- 确保ESP8266模块供电正常

## 📄 许可证

MIT License

## 🤝 贡献

欢迎提交Issue和Pull Request！

## 📧 联系方式

如有问题，请在GitHub仓库提交Issue。