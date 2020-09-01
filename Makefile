CC = sdcc

BUILD_DIR := build
TARGET    := $(BUILD_DIR)/blinky

include board/xm/board.mk

INCLUDE_DIRS := \
	/usr/share/sdcc/include \
	$(BOARD_DIR) \
	src \
	src/config

HEADERS := $(wildcard src/*.h)
SOURCES := src/main.c \
					 src/delay.c \
					 src/led.c \
					 src/uart.c \
					 src/timer.c \
					 src/radio.c \
					 src/dma.c

DEBUG_FLAGS = --verbose \
						  --debug      

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

REL=$(addsuffix .rel,$(addprefix $(BUILD_DIR)/,$(basename $(SOURCES))))

all: $(TARGET).bin

$(BUILD_DIR)/%.rel: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ -c $<

$(BUILD_DIR)/src/main.ihx: $(REL)
	$(CC) $(LDFLAGS) $(CFLAGS) -o $(BUILD_DIR)/src/main.ihx $(REL) 

$(TARGET).ihx: $(BUILD_DIR)/src/main.ihx
	@cp $< $@

$(TARGET).bin: $(TARGET).ihx
	objcopy -Iihex -Obinary $(TARGET).ihx $(TARGET).bin

clean:
	rm -rf $(BUILD_DIR)

.PHONY: clean 
