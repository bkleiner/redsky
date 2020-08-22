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
SOURCES := $(wildcard src/*.c)

CFLAGS  = --model-small \
					--opt-code-speed \
					$(addprefix -I ,$(INCLUDE_DIRS))

LDFLAGS = --out-fmt-ihx \
					--code-loc 0x0000 \
					--code-size $(FLASH_SIZE) \
					--xram-loc 0xf000 \
					--xram-size 0x300 \
					--iram-size 0x100

REL=$(addsuffix .rel,$(addprefix $(BUILD_DIR)/,$(basename $(SOURCES))))

all: $(TARGET).bin

$(BUILD_DIR)/%.rel: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ -c $<

$(TARGET).hex: $(REL) Makefile
	$(CC) $(LDFLAGS) $(CFLAGS) -o $(TARGET).hex $(REL) 

$(TARGET).bin: $(TARGET).hex
	objcopy -Iihex -Obinary $(TARGET).hex $(TARGET).bin

clean:
	rm -rf $(BUILD_DIR)

.PHONY: clean 
