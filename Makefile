BUILD_DIR := build
TARGET    ?= xm

include board/$(TARGET)/board.mk
include $(DRIVER_DIR)/driver.mk

INCLUDE_DIRS := \
	/usr/share/sdcc/include \
	$(BOARD_DIR) \
	$(DRIVER_DIR) \
	src/app \
	src/config

DRIVER_CORE_SOURCES := $(DRIVER_DIR)/clock.c \
											 $(DRIVER_DIR)/delay.c \
											 $(DRIVER_DIR)/flash.c \
											 $(DRIVER_DIR)/led.c \
											 $(DRIVER_DIR)/uart.c 

DRIVER_SOURCES := $(DRIVER_CORE_SOURCES) \
							 		$(DRIVER_DIR)/dma.c \
									$(DRIVER_DIR)/radio.c \
							 		$(DRIVER_DIR)/storage.c \
							 		$(DRIVER_DIR)/timer.c \
									$(DRIVER_DIR)/uart_dma.c

BOOTLOADER_SOURCES := src/bootloader/bootloader.c 

APP_SOURCES := src/app/app.c \
							 src/app/redpine.c \
							 src/app/debug.c

DRIVER_CORE_OBJECTS=$(addsuffix .$(OBJECT_EXT),$(addprefix $(BUILD_DIR)/,$(basename $(DRIVER_CORE_SOURCES))))
DRIVER_OBJECTS=$(addsuffix .$(OBJECT_EXT),$(addprefix $(BUILD_DIR)/,$(basename $(DRIVER_SOURCES))))

APP_OBJECTS=$(addsuffix .$(OBJECT_EXT),$(addprefix $(BUILD_DIR)/,$(basename $(APP_SOURCES))))
BOOTLOADER_OBJECTS=$(addsuffix .$(OBJECT_EXT),$(addprefix $(BUILD_DIR)/,$(basename $(BOOTLOADER_SOURCES))))

all: $(BUILD_DIR)/src/app.bin $(BUILD_DIR)/src/bootloader.bin

$(BUILD_DIR)/%.$(OBJECT_EXT): %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ -c $<

$(BUILD_DIR)/src/app.ihx: $(APP_OBJECTS) $(DRIVER_OBJECTS)
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/src/app.bin: $(BUILD_DIR)/src/app.ihx
	objcopy -Iihex -Obinary $< $@

$(BUILD_DIR)/src/bootloader.ihx: $(BOOTLOADER_OBJECTS) $(DRIVER_CORE_OBJECTS)
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/src/bootloader.bin: $(BUILD_DIR)/src/bootloader.ihx
	objcopy -Iihex -Obinary $< $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: clean 
