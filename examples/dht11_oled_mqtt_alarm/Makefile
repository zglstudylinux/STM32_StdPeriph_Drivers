# STM32 DHT11+OLED+MQTT Makefile
# Supports:
#   make              — compile
#   make clean        — remove artifacts
#
# Auto-detects ARM GCC from:
#   1. ARM_GCC_DIR env var
#   2. TOOLCHAIN_PATH in this Makefile
#   3. PATH

# Override by setting ARM_GCC_DIR environment variable
ifdef ARM_GCC_DIR
  TOOLCHAIN_PATH = $(ARM_GCC_DIR)
else
  TOOLCHAIN_PATH = C:/tools/arm-gcc/bin
endif

# Verify toolchain exists
TOOLCHAIN_CHECK := $(shell test -f $(TOOLCHAIN_PATH)/arm-none-eabi-gcc.exe || test -f $(TOOLCHAIN_PATH)/arm-none-eabi-gcc || which arm-none-eabi-gcc 2>/dev/null)
ifneq ($(TOOLCHAIN_CHECK),)
  # Fall back to PATH if the configured dir doesn't work
  TOOLCHAIN_PATH := $(shell dirname "$(shell which arm-none-eabi-gcc 2>/dev/null)")
endif

CC = $(TOOLCHAIN_PATH)/arm-none-eabi-gcc
AS = $(TOOLCHAIN_PATH)/arm-none-eabi-as
LD = $(TOOLCHAIN_PATH)/arm-none-eabi-gcc
OBJCOPY = $(TOOLCHAIN_PATH)/arm-none-eabi-objcopy

PROJECT_NAME = Project

CPU = -mcpu=cortex-m3
FPU =
CFLAGS = $(CPU) $(FPU) -mthumb -Wall -Wextra -Wno-missing-braces -ffunction-sections -fdata-sections -Os -std=c99 -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER
CFLAGS += -IStart -ILibrary -IUser -ISystem

LDSCRIPT = stm32f103c8t6.ld

SRCS = Start/startup_stm32f10x_md_gcc.s \
       Start/system_stm32f10x.c \
       Library/stm32f10x_gpio.c \
       Library/stm32f10x_rcc.c \
       Library/misc.c \
       Library/stm32f10x_usart.c \
       Library/stm32f10x_i2c.c \
       System/usart.c \
       System/dht11.c \
       System/oled.c \
       System/esp8266.c \
       System/delay.c \
       System/led_buzzer.c \
       User/main.c \
       User/stm32f10x_it.c

OBJS = $(SRCS:.s=.o)
OBJS := $(OBJS:.c=.o)

all: check_tools $(PROJECT_NAME).hex $(PROJECT_NAME).bin

check_tools:
	@if [ ! -x "$(CC)" ] && [ ! -f "$(CC).exe" ] && ! which arm-none-eabi-gcc >/dev/null 2>&1; then \
		echo "ERROR: ARM GCC not found at $(TOOLCHAIN_PATH)"; \
		echo "Set ARM_GCC_DIR env var or run: bash tools/setup_env.sh"; \
		exit 1; \
	fi

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
	rm -f Start/*.o Library/*.o System/*.o User/*.o
	rm -f $(PROJECT_NAME).elf $(PROJECT_NAME).hex $(PROJECT_NAME).bin $(PROJECT_NAME).map

.PHONY: all clean check_tools
