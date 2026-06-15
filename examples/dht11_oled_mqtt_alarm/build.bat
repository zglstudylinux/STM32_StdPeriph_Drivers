@echo off
setlocal enabledelayedexpansion

echo ========================================
echo   STM32 Project Builder
echo ========================================
echo.

REM ==================== 自动检测 ARM GCC ====================
set ARM_GCC_DIR=

REM 1. 检查环境变量
if defined ARM_GCC_DIR (
    if exist "!ARM_GCC_DIR!\arm-none-eabi-gcc.exe" goto :found_gcc
)

REM 2. 检查 PATH
where arm-none-eabi-gcc >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    for /f "delims=" %%i in ('where arm-none-eabi-gcc 2^>nul') do (
        set ARM_GCC_DIR=%%~dpi
        set ARM_GCC_DIR=!ARM_GCC_DIR:~0,-1!
        goto :found_gcc
    )
)

REM 3. 常见安装路径 (按优先级)
for %%d in (
    "C:\tools\arm-gcc\bin"
    "C:\Program Files (x86)\GNU Arm Embedded Toolchain\10 2021.10\bin"
    "C:\Program Files (x86)\GNU Arm Embedded Toolchain\10 2021.07\bin"
    "C:\Program Files (x86)\GNU Arm Embedded Toolchain\9 2020-q2-update\bin"
    "C:\Program Files (x86)\GNU Tools Arm Embedded\9 2019-q4-major\bin"
    "C:\ProgramData\chocolatey\bin"
) do (
    if exist "%%~d\arm-none-eabi-gcc.exe" (
        set ARM_GCC_DIR=%%~d
        goto :found_gcc
    )
)

REM 未找到
echo [错误] ARM GCC 工具链未找到！
echo.
echo 请先运行工具链检测脚本:
echo   bash tools/setup_env.sh
echo.
echo 或下载安装 ARM GNU Toolchain:
echo   https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads
echo.
echo 安装后，可以设置环境变量避免每次检测:
echo   setx ARM_GCC_DIR "C:\tools\arm-gcc\bin"
echo.
pause
exit /b 1

:found_gcc
echo [检测] ARM GCC: !ARM_GCC_DIR!
echo.

set CC=!ARM_GCC_DIR!\arm-none-eabi-gcc
set AS=!ARM_GCC_DIR!\arm-none-eabi-as
set LD=!ARM_GCC_DIR!\arm-none-eabi-gcc
set OBJCOPY=!ARM_GCC_DIR!\arm-none-eabi-objcopy

set CFLAGS=-mcpu=cortex-m3 -mthumb -Wall -Wextra -Wno-missing-braces -ffunction-sections -fdata-sections -Os -std=c99 -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER
set CFLAGS=%CFLAGS% -IStart -ILibrary -IUser -ISystem

set LDFLAGS=-mcpu=cortex-m3 -mthumb -Tstm32f103c8t6.ld -nostartfiles -specs=nano.specs -specs=nosys.specs -Wl,--gc-sections

echo [编译] 开始编译...
echo.

%CC% %CFLAGS% -c Start/system_stm32f10x.c -o Start/system_stm32f10x.o
if %ERRORLEVEL% neq 0 goto :error

%CC% %CFLAGS% -c Library/stm32f10x_gpio.c -o Library/stm32f10x_gpio.o
if %ERRORLEVEL% neq 0 goto :error

%CC% %CFLAGS% -c Library/stm32f10x_rcc.c -o Library/stm32f10x_rcc.o
if %ERRORLEVEL% neq 0 goto :error

%CC% %CFLAGS% -c Library/misc.c -o Library/misc.o
if %ERRORLEVEL% neq 0 goto :error

%CC% %CFLAGS% -c Library/stm32f10x_usart.c -o Library/stm32f10x_usart.o
if %ERRORLEVEL% neq 0 goto :error

%CC% %CFLAGS% -c Library/stm32f10x_i2c.c -o Library/stm32f10x_i2c.o
if %ERRORLEVEL% neq 0 goto :error

%CC% %CFLAGS% -c System/usart.c -o System/usart.o
if %ERRORLEVEL% neq 0 goto :error

%CC% %CFLAGS% -c System/dht11.c -o System/dht11.o
if %ERRORLEVEL% neq 0 goto :error

%CC% %CFLAGS% -c System/oled.c -o System/oled.o
if %ERRORLEVEL% neq 0 goto :error

%CC% %CFLAGS% -c System/esp8266.c -o System/esp8266.o
if %ERRORLEVEL% neq 0 goto :error

%CC% %CFLAGS% -c System/delay.c -o System/delay.o
if %ERRORLEVEL% neq 0 goto :error

%CC% %CFLAGS% -c System/led_buzzer.c -o System/led_buzzer.o
if %ERRORLEVEL% neq 0 goto :error

%CC% %CFLAGS% -c User/main.c -o User/main.o
if %ERRORLEVEL% neq 0 goto :error

%CC% %CFLAGS% -c User/stm32f10x_it.c -o User/stm32f10x_it.o
if %ERRORLEVEL% neq 0 goto :error

%AS% -mcpu=cortex-m3 -mthumb Start/startup_stm32f10x_md_gcc.s -o Start/startup_stm32f10x_md_gcc.o
if %ERRORLEVEL% neq 0 goto :error

echo.
echo [链接] 生成固件...
echo.

%LD% %LDFLAGS% Start/startup_stm32f10x_md_gcc.o Start/system_stm32f10x.o Library/stm32f10x_gpio.o Library/stm32f10x_rcc.o Library/misc.o Library/stm32f10x_usart.o Library/stm32f10x_i2c.o System/usart.o System/dht11.o System/oled.o System/esp8266.o System/delay.o System/led_buzzer.o User/main.o User/stm32f10x_it.o -o Project.elf

if %ERRORLEVEL% neq 0 goto :error

echo [生成] HEX 和 BIN 文件...
echo.

%OBJCOPY% -O ihex Project.elf Project.hex
%OBJCOPY% -O binary Project.elf Project.bin

echo.
echo ========================================
echo   [成功] 编译完成！
echo ========================================
echo.
echo 产物:
echo   Project.elf  (ELF 可执行文件)
echo   Project.hex  (HEX 烧录文件)
echo   Project.bin  (原始二进制)
echo.
echo 下一步:
echo   build_and_flash.bat   (编译+烧录+监控)
echo   bash build_flash_monitor.sh flash   (命令行烧录)
echo.
goto :end

:error
echo.
echo [失败] 编译出错！
echo.
pause
exit /b 1

:end
endlocal
