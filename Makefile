CC = sdcc

BUILD_DIR := build
TARGET    ?= xm

include board/$(TARGET)/board.mk

INCLUDE_DIRS := \
	/usr/share/sdcc/include \
	$(BOARD_DIR) \
	src \
	src/config \
	src/core

CORE_SOURCES := src/core/led.c \
								src/core/clock.c \
								src/core/flash.c \
								src/core/delay.c \
								src/core/uart.c 

BOOTLOADER_SOURCES := src/bootloader/bootloader.c 

APP_SOURCES := src/app/app.c \
							 src/app/uart_dma.c \
							 src/app/timer.c \
							 src/app/radio.c \
							 src/app/debug.c \
							 src/app/dma.c \
							 src/app/redpine.c

DEBUG_FLAGS = --debug \
							--verbose

CFLAGS  = $(DEBUG_FLAGS) \
					--model-small \
					--opt-code-speed \
					$(addprefix -I ,$(INCLUDE_DIRS)) 

LDFLAGS = --out-fmt-ihx \
					--code-loc 0x0000 \
					--code-size $(FLASH_SIZE) \
					--xram-loc 0xf000 \
					--xram-size 0x7FF \
					--iram-size 0x100

CORE_REL=$(addsuffix .rel,$(addprefix $(BUILD_DIR)/,$(basename $(CORE_SOURCES))))
APP_REL=$(addsuffix .rel,$(addprefix $(BUILD_DIR)/,$(basename $(APP_SOURCES))))
BOOTLOADER_REL=$(addsuffix .rel,$(addprefix $(BUILD_DIR)/,$(basename $(BOOTLOADER_SOURCES))))

all: $(BUILD_DIR)/src/app.bin $(BUILD_DIR)/src/bootloader.bin

$(BUILD_DIR)/%.rel: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ -c $<

$(BUILD_DIR)/src/app.ihx: $(APP_REL) $(CORE_REL)
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/src/app.bin: $(BUILD_DIR)/src/app.ihx
	objcopy -Iihex -Obinary $< $@

$(BUILD_DIR)/src/bootloader.ihx: $(BOOTLOADER_REL) $(CORE_REL)
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/src/bootloader.bin: $(BUILD_DIR)/src/bootloader.ihx
	objcopy -Iihex -Obinary $< $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: clean 
