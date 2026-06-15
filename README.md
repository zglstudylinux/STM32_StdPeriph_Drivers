# STM32_StdPeriph_Drivers

基于STM32F10x标准外设库的嵌入式开发项目模板，包含常用传感器驱动、显示模块和通信组件。

## 项目结构

```
STM32_StdPeriph_Drivers/
├── docs/                    # 文档目录
├── drivers/                 # 外设驱动目录
│   ├── stm32f10x_std/       # STM32F10x标准库驱动
│   ├── sensor/              # 传感器驱动
│   ├── display/             # 显示模块驱动
│   └── communication/       # 通信模块驱动
├── examples/                # 示例项目
├── tools/                   # 工具脚本
└── common/                  # 公共组件
```

## 目录说明

| 目录 | 说明 | 状态 |
|------|------|------|
| `drivers/stm32f10x_std/` | STM32F10x标准外设库 | ✅ |
| `drivers/sensor/` | 传感器驱动（DHT11等） | ✅ |
| `drivers/display/` | 显示模块驱动（OLED等） | ✅ |
| `drivers/communication/` | 通信模块驱动（ESP8266等） | ✅ |
| `examples/` | 示例项目集合 | ✅ |
| `common/` | 公共组件（Delay等） | ✅ |
| `tools/` | 辅助工具脚本 | ✅ |
| `docs/` | 项目文档 | ✅ |

## 已实现驱动

### 传感器驱动
- **DHT11** - 温湿度传感器驱动
  - 引脚：PB15
  - 通信协议：单总线

### 显示驱动
- **OLED 0.96寸** - SSD1306驱动
  - 引脚：PB8(SCL), PB9(SDA)
  - 通信协议：I2C

### 通信驱动
- **ESP8266** - WiFi模块驱动
  - 引脚：PA2(TX), PA3(RX)
  - 通信协议：UART + AT指令
  - 支持：WiFi连接、TCP通信

## 示例项目

### 1. dht11_oled_mqtt
- **功能**：温湿度采集 + OLED显示 + TCP数据上传
- **硬件需求**：
  - STM32F103C8T6开发板
  - DHT11温湿度传感器
  - 0.96寸OLED显示屏
  - ESP8266 WiFi模块

## 快速开始

### 环境要求
- GCC ARM Embedded Toolchain
- OpenOCD (for flashing)
- Python 3.x (for tools)

### 编译项目
```bash
cd examples/dht11_oled_mqtt
make
```

### 烧录程序
```bash
openocd -f stlink.cfg -f stm32f1x_custom.cfg -c "program Project.hex reset exit"
```

### 启动串口监控
```bash
python tools/serial_monitor.py
```

### 启动上位机
```bash
python tools/mqtt_monitor.py
```

## 硬件连接

### STM32F103C8T6引脚分配

| 功能 | 引脚 | 说明 |
|------|------|------|
| DHT11 DATA | PB15 | 温湿度传感器数据引脚 |
| OLED SCL | PB8 | I2C时钟线 |
| OLED SDA | PB9 | I2C数据线 |
| ESP8266 TX | PA3 | USART2接收 |
| ESP8266 RX | PA2 | USART2发送 |
| USART1 TX | PA9 | 调试串口输出 |
| USART1 RX | PA10 | 调试串口输入 |

### DHT11连接
```
DHT11模块        STM32
VCC  ----------  3.3V
GND  ----------  GND
DATA ----------  PB15
```

### OLED连接
```
OLED模块         STM32
VCC  ----------  3.3V
GND  ----------  GND
SCL  ----------  PB8
SDA  ----------  PB9
```

### ESP8266连接
```
ESP8266模块      STM32
VCC  ----------  3.3V
GND  ----------  GND
TX   ----------  PA3
RX   ----------  PA2
```

## API参考

### DHT11驱动

```c
// 初始化
void DHT11_Init(void);

// 读取温湿度数据
uint8_t DHT11_ReadData(DHT11_Data *data);

// 数据结构
typedef struct {
    uint8_t humidity;        // 湿度整数部分
    uint8_t humidity_dec;    // 湿度小数部分
    uint8_t temperature;     // 温度整数部分
    uint8_t temperature_dec; // 温度小数部分
    uint8_t checksum;        // 校验和
} DHT11_Data;
```

### OLED驱动

```c
// 初始化
void OLED_Init(void);

// 清屏
void OLED_Clear(void);

// 显示字符
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char);

// 显示字符串
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String);

// 显示数字
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
```

### ESP8266驱动

```c
// 初始化
void ESP8266_Init(uint32_t baudrate);

// 连接WiFi
ESP8266_Status ESP8266_ConnectWiFi(const char *ssid, const char *password);

// 连接TCP服务器
ESP8266_Status ESP8266_ConnectTCP(const char *server, uint16_t port);

// 发送TCP数据
ESP8266_Status ESP8266_SendTCPData(const char *data);

// 断开TCP连接
ESP8266_Status ESP8266_DisconnectTCP(void);
```

## 配置说明

### WiFi配置
在 `main.c` 中修改以下宏定义：
```c
#define WIFI_SSID "your_ssid"
#define WIFI_PASSWORD "your_password"
```

### 服务器配置
```c
#define SERVER_IP "192.168.0.85"  // 上位机IP地址
#define SERVER_PORT 1883           // 服务器端口
```

## 故障排除

### 常见问题

1. **DHT11读取失败**
   - 检查PB15引脚连接
   - 确保传感器供电正常
   - 检查传感器模块是否正常

2. **OLED不显示**
   - 检查PB8、PB9引脚连接
   - 检查I2C地址是否正确（默认0x78）
   - 确保OLED模块供电正常

3. **WiFi连接失败**
   - 检查ESP8266模块连接
   - 确保WiFi SSID和密码正确
   - 检查串口波特率设置（默认115200）

4. **TCP连接失败**
   - 检查上位机IP地址配置
   - 确保上位机服务器已启动
   - 检查网络是否在同一网段

## 许可证

MIT License

## 贡献

欢迎提交Issue和Pull Request！

---

**作者**: zglstudylinux  
**仓库**: [GitHub](https://github.com/zglstudylinux/STM32_StdPeriph_Drivers)