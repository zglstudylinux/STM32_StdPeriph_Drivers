TOOLCHAIN_PATH = C:/tools/arm-gcc/bin
CC = $(TOOLCHAIN_PATH)/arm-none-eabi-gcc
AS = $(TOOLCHAIN_PATH)/arm-none-eabi-as
LD = $(TOOLCHAIN_PATH)/arm-none-eabi-gcc
OBJCOPY = $(TOOLCHAIN_PATH)/arm-none-eabi-objcopy

PROJECT_NAME = Project

CPU = -mcpu=cortex-m3
FPU =
CFLAGS = $(CPU) $(FPU) -mthumb -Wall -Wextra -ffunction-sections -fdata-sections -Os -std=c99 -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER
CFLAGS += -IStart -ILibrary -IUser -ISystem

LDSCRIPT = stm32f103c8t6.ld

SRCS = Start/startup_stm32f10x_md_gcc.s Start/system_stm32f10x.c Library/stm32f10x_gpio.c Library/stm32f10x_rcc.c Library/misc.c Library/stm32f10x_usart.c Library/stm32f10x_i2c.c System/USART.c System/DHT11.c System/OLED.c System/ESP8266.c System/Delay.c User/main.c User/stm32f10x_it.c

OBJS = $(SRCS:.s=.o)
OBJS := $(OBJS:.c=.o)

all: $(PROJECT_NAME).hex $(PROJECT_NAME).bin

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(AS) $(CPU) -mthumb $< -o $@

$(PROJECT_NAME).elf: $(OBJS)
	$(LD) $(CPU) -mthumb -T$(LDSCRIPT) -nostartfiles -specs=nano.specs -specs=nosys.specs -Wl,--gc-sections -Wl,-Map=$(PROJECT_NAME).map $^ -o $@

$(PROJECT_NAME).hex: $(PROJECT_NAME).elf
	$(OBJCOPY) -O ihex $< $@

$(PROJECT_NAME).bin: $(PROJECT_NAME).elf
	$(OBJCOPY) -O binary $< $@

clean:
	del /Q Start\*.o Library\*.o System\*.o User\*.o $(PROJECT_NAME).elf $(PROJECT_NAME).hex $(PROJECT_NAME).bin $(PROJECT_NAME).map 2>NUL

.PHONY: all clean
