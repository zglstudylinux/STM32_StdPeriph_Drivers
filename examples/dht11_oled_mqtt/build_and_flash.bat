@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

echo ========================================
echo   STM32 编译烧录测试自动化脚本
echo ========================================
echo.

REM ==================== 自动检测 OpenOCD ====================
set OPENOCD_PATH=

REM 1. 检查环境变量
if defined OPENOCD_PATH (
    if exist "!OPENOCD_PATH!" goto :found_openocd
)

REM 2. 检查 PATH
where openocd >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    for /f "delims=" %%i in ('where openocd 2^>nul') do (
        set OPENOCD_PATH=%%i
        goto :found_openocd
    )
)

REM 3. 常见安装路径
for %%d in (
    "C:\tools\openocd\xpack-openocd-0.12.0-3\bin\openocd.exe"
    "C:\tools\openocd\xpack-openocd-0.12.0-2\bin\openocd.exe"
    "C:\tools\openocd\xpack-openocd-0.11.0-1\bin\openocd.exe"
    "C:\OpenOCD\bin\openocd.exe"
    "C:\Program Files\OpenOCD\bin\openocd.exe"
    "C:\ProgramData\chocolatey\bin\openocd.exe"
) do (
    if exist %%d (
        set OPENOCD_PATH=%%d
        goto :found_openocd
    )
)

echo [警告] OpenOCD 未找到
echo   下载地址: https://gnutoolchains.com/arm-eabi/openocd/
echo   或者运行: bash tools/setup_env.sh 自动检测
echo.
echo [跳过] 跳过烧录步骤，仅编译项目...
echo.
set SKIP_FLASH=1
goto :skip_flash

:found_openocd
REM Derive scripts path from openocd location
for %%f in ("!OPENOCD_PATH!") do set OCD_BIN_DIR=%%~dpf
set OPENOCD_SCRIPTS=!OCD_BIN_DIR!..\scripts

REM If not found at ../scripts, try common location
if not exist "!OPENOCD_SCRIPTS!\interface\stlink.cfg" (
    set OPENOCD_SCRIPTS=C:\tools\openocd\xpack-openocd-0.12.0-3\scripts
)

echo [检测] OpenOCD: !OPENOCD_PATH!
if exist "!OPENOCD_SCRIPTS!\interface\stlink.cfg" (
    echo [检测] Scripts: !OPENOCD_SCRIPTS!
) else (
    echo [警告] OpenOCD scripts 目录未正确识别，可能需要手动指定
)
echo.

:skip_flash

REM ==================== 步骤1: 编译 ====================
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

REM 检查 HEX 文件
if not exist "Project.hex" (
    echo [错误] HEX 文件未生成
    pause
    exit /b 1
)

REM 如果跳过烧录，直接到串口监控
if "%SKIP_FLASH%"=="1" goto :serial_monitor

REM ==================== 步骤2: 烧录 ====================
echo [步骤2] 烧录程序到 STM32...
echo.
echo 请确保：
echo   1. STM32 开发板已连接
echo   2. ST-Link 调试器已连接
echo   3. 开发板已上电
echo.

"!OPENOCD_PATH!" -s "!OPENOCD_SCRIPTS!" -f interface/stlink.cfg -f stm32f1x_custom.cfg -c "program Project.hex verify reset exit"

if %ERRORLEVEL% neq 0 (
    echo.
    echo [错误] 烧录失败！
    echo 请检查：
    echo   1. ST-Link USB 线是否插好
    echo   2. STM32 是否上电
    echo   3. SWCLK/SWDIO/GND 接线是否正确
    echo.
    pause
    exit /b 1
)

echo.
echo [成功] 烧录完成！
echo.

REM ==================== 步骤3: 串口监控 ====================
:serial_monitor
echo [步骤3] 启动串口监控...
echo.
echo 按 Ctrl+C 停止监控
echo.
echo ========================================
echo.

REM 尝试自动检测串口
python -c "import serial.tools.list_ports; ports=list(serial.tools.list_ports.comports()); print('\n'.join([p.device for p in ports]) if ports else '')" > %TEMP%\stm32_ports.txt 2>nul
set /p DETECTED_PORT=<%TEMP%\stm32_ports.txt 2>nul

if not "%DETECTED_PORT%"=="" (
    echo [检测] 发现串口: %DETECTED_PORT%
    echo.
    python "..\..\tools\serial_monitor.py" %DETECTED_PORT%
) else (
    echo [提示] 未自动检测到串口，使用默认 COM3
    echo 可使用 python tools\serial_monitor.py --list 查看可用串口
    echo.
    python "..\..\tools\serial_monitor.py"
)

echo.
echo [完成] 所有步骤已完成
echo.
pause
