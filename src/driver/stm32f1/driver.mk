CC = arm-none-eabi-gcc
CP = arm-none-eabi-objcopy

OBJECT_EXT = o
TARGET_EXT = elf

CFLAGS = -g -O1 -Wall -Wextra \
				 -mthumb -mcpu=cortex-m3 \
				 --specs=nano.specs --specs=nosys.specs \
				 -DSTM32F103xB

CMSIS_DIR = third_party/STM32CubeF1/Drivers/CMSIS
CMSIS_DEVICE_DIR = $(CMSIS_DIR)/Device/ST/STM32F1xx

LD_SCRIPT_DIR = $(CMSIS_DEVICE_DIR)/Source/Templates/gcc/linker/

LDFLAGS = -static -lc -lnosys -lm \
					-Wl,-L$(LD_SCRIPT_DIR),-T$(LD_SCRIPT_DIR)/STM32F103XB_FLASH.ld \
					-Wl,-gc-sections

DRIVER_CORE_SOURCES := $(DRIVER_DIR)/delay.c \
											 $(DRIVER_DIR)/led.c \
											 $(DRIVER_DIR)/uart.c \
											 $(DRIVER_DIR)/clock.c \
											 $(DRIVER_DIR)/gpio.c \
											 $(CMSIS_DEVICE_DIR)/Source/Templates/gcc/startup_stm32f103xb.s \
											 $(DRIVER_DIR)/system_stm32f1xx.c

DRIVER_SOURCES := $(DRIVER_CORE_SOURCES) \
									$(DRIVER_DIR)/dma.c \
								  $(DRIVER_DIR)/storage.c \
									$(DRIVER_DIR)/timer.c \
									$(DRIVER_DIR)/uart_dma.c \
									$(DRIVER_DIR)/crc.c \
									$(DRIVER_DIR)/radio.c

DRIVER_INCLUDE_DIRS := $(DRIVER_DIR) \
											 $(CMSIS_DIR)/Include \
											 $(CMSIS_DEVICE_DIR)/Include