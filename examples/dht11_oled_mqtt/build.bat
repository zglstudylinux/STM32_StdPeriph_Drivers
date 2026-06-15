@echo off
setlocal

set TOOLCHAIN=C:\tools\arm-gcc\bin
set CC=%TOOLCHAIN%\arm-none-eabi-gcc
set AS=%TOOLCHAIN%\arm-none-eabi-as
set LD=%TOOLCHAIN%\arm-none-eabi-gcc
set OBJCOPY=%TOOLCHAIN%\arm-none-eabi-objcopy

set CFLAGS=-mcpu=cortex-m3 -mthumb -Wall -Wextra -ffunction-sections -fdata-sections -Os -std=c99 -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER
set CFLAGS=%CFLAGS% -IStart -ILibrary -IUser -ISystem

set LDFLAGS=-mcpu=cortex-m3 -mthumb -Tstm32f103c8t6.ld -nostartfiles -specs=nano.specs -specs=nosys.specs -Wl,--gc-sections

echo Compiling...

%CC% %CFLAGS% -c Start/system_stm32f10x.c -o Start/system_stm32f10x.o
%CC% %CFLAGS% -c Library/stm32f10x_gpio.c -o Library/stm32f10x_gpio.o
%CC% %CFLAGS% -c Library/stm32f10x_rcc.c -o Library/stm32f10x_rcc.o
%CC% %CFLAGS% -c Library/misc.c -o Library/misc.o
%CC% %CFLAGS% -c Library/stm32f10x_usart.c -o Library/stm32f10x_usart.o
%CC% %CFLAGS% -c Library/stm32f10x_i2c.c -o Library/stm32f10x_i2c.o
%CC% %CFLAGS% -c System/USART.c -o System/USART.o
%CC% %CFLAGS% -c System/DHT11.c -o System/DHT11.o
%CC% %CFLAGS% -c System/OLED.c -o System/OLED.o
%CC% %CFLAGS% -c System/ESP8266.c -o System/ESP8266.o
%CC% %CFLAGS% -c System/Delay.c -o System/Delay.o
%CC% %CFLAGS% -c User/main.c -o User/main.o
%CC% %CFLAGS% -c User/stm32f10x_it.c -o User/stm32f10x_it.o
%AS% -mcpu=cortex-m3 -mthumb Start/startup_stm32f10x_md_gcc.s -o Start/startup_stm32f10x_md_gcc.o

echo Linking...

%LD% %LDFLAGS% Start/startup_stm32f10x_md_gcc.o Start/system_stm32f10x.o Library/stm32f10x_gpio.o Library/stm32f10x_rcc.o Library/misc.o Library/stm32f10x_usart.o Library/stm32f10x_i2c.o System/USART.o System/DHT11.o System/OLED.o System/ESP8266.o System/Delay.o User/main.o User/stm32f10x_it.o -o Project.elf

echo Generating HEX and BIN...

%OBJCOPY% -O ihex Project.elf Project.hex
%OBJCOPY% -O binary Project.elf Project.bin

echo Build completed successfully!
