CC = sdcc
AS = sdas8051
CP = objcopy

OBJECT_EXT = rel
TARGET_EXT = ihx

DEBUG_FLAGS = --debug

CFLAGS  = $(DEBUG_FLAGS) \
					--model-small \
					--opt-code-speed

LDFLAGS = --out-fmt-ihx \
					--xram-loc 0xf000 \
					--xram-size 0x7FF \
					--iram-size 0x100

ASFLAGS = -plosgff

BOOTLOADER_LDFLAGS = $(LDFLAGS) \
										 --code-loc 0x0 \
										 --code-size $(BOOTLOADER_SIZE)

APP_LDFLAGS = $(LDFLAGS) \
							--code-loc $(BOOTLOADER_SIZE) \
							--code-size $(FLASH_SIZE)

CP_FLAGS = -Iihex

DRIVER_CORE_SOURCES := $(DRIVER_DIR)/led.c \
											 $(DRIVER_DIR)/clock.c \
											 $(DRIVER_DIR)/delay.c \
											 $(DRIVER_DIR)/flash.c \
											 $(DRIVER_DIR)/uart.c 

DRIVER_SOURCES := $(DRIVER_CORE_SOURCES) \
							 		$(DRIVER_DIR)/dma.c \
									$(DRIVER_DIR)/radio.c \
							 		$(DRIVER_DIR)/storage.c \
							 		$(DRIVER_DIR)/timer.c \
									$(DRIVER_DIR)/uart_dma.c \
									$(DRIVER_DIR)/crc.c

BOOTLOADER_SOURCES := $(DRIVER_DIR)/startup.asm

DRIVER_INCLUDE_DIRS := $(DRIVER_DIR)