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
| 调试器 | ST-Link V2/V3 | 1 |

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

| 工具 | 用途 | 下载 |
|------|------|------|
| ARM GCC (14.2+) | 交叉编译器 | [GNU Arm Embedded](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads) |
| OpenOCD | 烧录/调试 | [gnutoolchains.com](https://gnutoolchains.com/arm-eabi/openocd/) 或 [xPack](https://github.com/xpack-dev-tools/openocd-xpack/releases) |
| Make (可选) | 构建工具 | [MinGW](https://sourceforge.net/projects/mingw/) |
| Python 3.x | 上位机 | [python.org](https://www.python.org/downloads/) |
| pyserial | 串口通信 | `pip install pyserial` |

### 0. 环境检测（换电脑后第一步）

项目内置了工具链自动检测脚本，**无需手动配置环境变量**：

```bash
# 自动检测所有工具链
bash tools/setup_env.sh
```

输出示例 — 每项 OK 即为就绪，MISS 会给出下载地址：

```
--- ARM GCC Toolchain ---
  [OK]  arm-none-eabi-gcc 14.2.1
  [OK]  Location: /c/tools/arm-gcc/bin

--- OpenOCD ---
  [OK]  OpenOCD 0.12.0
  [OK]  Scripts: /c/tools/openocd/.../scripts

--- Python ---
  [OK]  Python 3.12.7
  [OK]  pyserial module

--- Make ---
  [OK]  GNU Make 3.81

All required tools found! Ready to build.
```

**检测逻辑**：按 `PATH → 环境变量 → 常见安装目录` 的优先级搜索，结果缓存到 `tools/tools_paths.conf`。之后 `build_flash_monitor.sh`、`build.bat` 都会自动读取，**你再也不用手动改路径了**。

> 如果某工具未安装，脚本会给出精确的下载链接。安装后重新运行 `bash tools/setup_env.sh` 即可。

## 快速开始

### 1. 配置 WiFi 和服务器地址

编辑 `User/main.c` 中的宏定义：

```c
#define WIFI_SSID      "你的WiFi名称"
#define WIFI_PASSWORD  "你的WiFi密码"
#define SERVER_IP      "上位机IP地址"   // 运行上位机的电脑IP
#define SERVER_PORT    1883
```

### 2. 安装 Python 依赖

```bash
pip install pyserial
```

### 3. 编译项目

```bash
cd examples/dht11_oled_mqtt

# Windows CMD
build.bat

# Linux/Mac/Git Bash
bash build_flash_monitor.sh compile
```

### 4. 烧录程序

```bash
# 自动化烧录
bash build_flash_monitor.sh flash

# 或手动烧录
openocd -f interface/stlink.cfg -f stm32f1x_custom.cfg -c "program Project.hex verify reset exit"
```

### 5. 启动上位机（⚠️ 重要：必须在 STM32 之前启动）

```bash
# 启动 TCP 服务器 GUI（监听 1883 端口）
python tools/mqtt_monitor.py &

# 再启动串口监控（默认 COM3，根据实际情况修改）
python tools/serial_monitor.py COM8
```

> **⚠️ 关键：先启动上位机，再给 STM32 上电/复位！**  
> STM32 上电后会立即尝试连接 TCP 服务器。如果上位机未启动，TCP 连接会失败（显示 `TCP Connect Failed!`）。启动上位机后，下一次采样周期即可自动恢复连接。

### 6. 查看数据

串口输出格式：

```
=== System Init ===
DHT11 Init OK!
OLED Init OK!
ESP8266 Init OK!
WiFi Connected!

=== Data #1 ===
Humidity: 75.0%
Temperature: 28.1C
TCP Connected!
Data Send OK!
```

TCP 数据格式（JSON）：

```json
{"temperature":28.1,"humidity":75.0,"count":1}
```

## 一键自动化脚本

项目提供了 `build_flash_monitor.sh` 脚本，**首次使用前先运行 `bash tools/setup_env.sh` 检测工具链**，之后脚本自动找到编译器、烧录器等工具，支持多种工作模式：

```bash
cd examples/dht11_oled_mqtt

# 仅编译（自动检测 ARM GCC）
bash build_flash_monitor.sh compile

# 编译 + 烧录（自动检测 OpenOCD + ST-Link）
bash build_flash_monitor.sh flash

# 编译 + 烧录 + 串口监控（自动检测串口）
bash build_flash_monitor.sh monitor

# 完整工作流：编译 + 烧录 + 上位机 + 串口监控
bash build_flash_monitor.sh all

# 自定义串口号
SERIAL_PORT=COM5 bash build_flash_monitor.sh monitor

# 清理编译产物
bash build_flash_monitor.sh clean
```

> **自动检测原理**：脚本优先读取 `tools/tools_paths.conf`（由 `setup_env.sh` 生成），若不存在则临时搜索常见路径。Windows 用户也可用 `build.bat` / `build_and_flash.bat`，内置同样的多级路径搜索逻辑。

## 完整操作流程（推荐）

按以下顺序操作，确保所有功能正常：

```
1. [首次] 检测工具链：bash tools/setup_env.sh
2. 连接硬件（ST-Link + USB转TTL + DHT11 + OLED + ESP8266）
3. 修改 User/main.c 中的 WiFi 和服务器 IP 配置
4. 编译 + 烧录固件：bash examples/dht11_oled_mqtt/build_flash_monitor.sh flash
5. 先启动上位机：python tools/mqtt_monitor.py &
6. 再复位 STM32（或重新上电）
7. 启动串口监控：python tools/serial_monitor.py COM8
8. 观察数据上报
```

> **换电脑只需重做第1步**，运行 `bash tools/setup_env.sh` 重新检测即可，无需手动改任何路径。

## 固件信息

| 项目 | 数值 |
|------|------|
| MCU | STM32F103C8T6 |
| Flash占用 | ~9KB / 64KB |
| 编译器 | ARM GCC 14.2.1 |
| 优化等级 | -Os |
| 采样周期 | 3秒 |

## 项目结构

```
STM32_StdPeriph_Drivers/
├── tools/                       # 工具脚本
│   ├── setup_env.sh            # 工具链自动检测（新环境跑一次）
│   ├── tools_paths.conf        # 检测结果缓存（机器相关，不提交git）
│   ├── serial_monitor.py       # 串口监控工具
│   └── mqtt_monitor.py         # TCP上位机GUI
│
└── examples/dht11_oled_mqtt/
    ├── Library/                 # STM32标准库
    ├── Start/                   # 启动文件
    ├── System/                  # 驱动代码
    │   ├── dht11.c/h           # DHT11温湿度驱动
    │   ├── oled.c/h            # OLED显示驱动
    │   ├── oled_font.h         # OLED字库
    │   ├── esp8266.c/h         # ESP8266 WiFi驱动
    │   ├── usart.c/h           # USART串口驱动
    │   └── delay.c/h           # 延时函数
    ├── User/                    # 用户代码
    │   ├── main.c              # 主程序
    │   ├── stm32f10x_conf.h    # 标准库配置
    │   └── stm32f10x_it.c/h   # 中断服务
    ├── Makefile                 # Makefile 编译配置
    ├── build.bat                # Windows 编译脚本
    ├── build_and_flash.bat      # Windows 编译+烧录+监控脚本
    ├── build_flash_monitor.sh   # 跨平台一键脚本 (推荐)
    ├── stm32f103c8t6.ld        # 链接脚本
    └── stm32f1x_custom.cfg     # OpenOCD 配置
```

## 故障排除

### 编译错误
- **`arm-none-eabi-gcc: command not found`**: 运行 `bash tools/setup_env.sh` 自动检测工具链位置。如果未安装，脚本会给出下载地址
- **`make: command not found`**: 直接用 `build.bat` 或 `bash build_flash_monitor.sh compile` 代替，不依赖 make
- **换了一台电脑编译失败**: 先运行 `bash tools/setup_env.sh` 重新生成 `tools/tools_paths.conf`，不同电脑的工具链路径可能不同

### 烧录错误
- **`init mode failed`**: 检查ST-Link连接，确保驱动已安装
- **`UNEXPECTED idcode`**: 项目已配置自定义 `stm32f1x_custom.cfg`，无需额外操作

### WiFi连接失败
1. 检查 `WIFI_SSID` 和 `WIFI_PASSWORD` 是否正确
2. 确保ESP8266模块供电正常（3.3V）
3. 检查CH_PD引脚是否接3.3V
4. 确认WiFi是2.4GHz频段（ESP8266不支持5GHz）

### TCP连接失败 (`TCP Connect Failed!`)
1. **常见原因：上位机未启动** — STM32尝试连接TCP服务器时，如果上位机未运行，会显示此错误
2. **解决方法：先启动上位机 `python tools/mqtt_monitor.py &`，再复位STM32**
3. 确认 `SERVER_IP` 是上位机电脑的IP地址（运行 `ipconfig` 查看）
4. 确保STM32和上位机在**同一局域网**内
5. 检查防火墙是否阻止了 1883 端口

### DHT11读取失败
1. 检查DATA引脚连接（PB15）
2. 确保外接10K上拉电阻
3. 检查传感器是否正常

### 串口无法打开 (`PermissionError`)
1. 检查是否有其他程序占用了串口（Arduino IDE、Putty、串口助手等）
2. 关闭占用程序后重试
3. 使用 `python tools/serial_monitor.py --list` 查看可用串口列表

### 串口号不是COM3
- 使用 `python tools/serial_monitor.py --list` 列出所有可用串口
- 指定正确的串口号：`python tools/serial_monitor.py COM8`

## 数据流架构

```
DHT11 ──┬──> OLED (SSD1306 I2C)    实时显示温湿度
        │
        ├──> USART1 (PA9/PA10)      串口调试输出
        │       └──> 上位机串口监控
        │
        └──> USART2 (PA2/PA3)       ESP8266 WiFi模块
                └──> TCP (1883) ──> 上位机 GUI 显示
```

## 许可证

MIT License
