@echo off
chcp 65001 >nul
echo ========================================
echo   STM32 编译烧录测试自动化脚本
echo ========================================
echo.

REM 设置OpenOCD路径
set OPENOCD_PATH=C:\tools\openocd\xpack-openocd-0.12.0-3\bin\openocd.exe
set OPENOCD_SCRIPTS=C:\tools\openocd\xpack-openocd-0.12.0-3\scripts

REM 检查OpenOCD是否存在
if not exist "%OPENOCD_PATH%" (
    echo [错误] OpenOCD未找到: %OPENOCD_PATH%
    echo 请确保OpenOCD已安装并配置正确路径
    echo.
    echo 可从以下地址下载：
    echo https://gnutoolchains.com/arm-eabi/openocd/
    pause
    exit /b 1
)

echo [步骤1] 编译项目...
echo.
call build.bat

if %ERRORLEVEL% neq 0 (
    echo.
    echo [错误] 编译失败！
    pause
    exit /b 1
)

echo.
echo [成功] 编译完成！
echo.

REM 检查HEX文件
if not exist "Project.hex" (
    echo [错误] HEX文件未生成
    pause
    exit /b 1
)

echo [步骤2] 烧录程序到STM32...
echo.
echo 请确保：
echo   1. STM32开发板已连接
echo   2. ST-Link调试器已连接
echo   3. 开发板已上电
echo.

"%OPENOCD_PATH%" -s "%OPENOCD_SCRIPTS%" -f interface/stlink.cfg -f stm32f1x_custom.cfg -c "program Project.hex reset exit"

if %ERRORLEVEL% neq 0 (
    echo.
    echo [错误] 烧录失败！
    echo 请检查硬件连接和ST-Link驱动
    pause
    exit /b 1
)

echo.
echo [成功] 烧录完成！
echo.

echo [步骤3] 启动串口监控...
echo.
echo 即将启动串口监控程序
echo 波特率: 115200
echo 按 Ctrl+C 停止监控
echo.
echo ========================================
echo.

REM 返回上级目录并启动串口监控
cd ..\..\tools
start python serial_monitor.py

echo [完成] 串口监控已启动
echo.
pause
