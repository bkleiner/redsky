BUILD_DIR := build
TARGET    ?= d8r

include board/$(TARGET)/board.mk
include $(DRIVER_DIR)/driver.mk

INCLUDE_DIRS := $(BOARD_DIR) \
								$(DRIVER_INCLUDE_DIRS) \
								src/app \
								src/config

CFLAGS += $(addprefix -I ,$(INCLUDE_DIRS)) 

BOOTLOADER_SOURCES := src/bootloader/bootloader.c 
APP_SOURCES := src/app/app.c \
							 src/app/redpine.c \
							 src/app/debug.c

DRIVER_CORE_OBJECTS=$(addsuffix .$(OBJECT_EXT),$(addprefix $(BUILD_DIR)/,$(basename $(DRIVER_CORE_SOURCES))))
DRIVER_OBJECTS=$(addsuffix .$(OBJECT_EXT),$(addprefix $(BUILD_DIR)/,$(basename $(DRIVER_SOURCES))))

BOOTLOADER_OBJECTS=$(addsuffix .$(OBJECT_EXT),$(addprefix $(BUILD_DIR)/,$(basename $(BOOTLOADER_SOURCES))))
APP_OBJECTS=$(addsuffix .$(OBJECT_EXT),$(addprefix $(BUILD_DIR)/,$(basename $(APP_SOURCES))))

all: bootloader app

bootloader: $(BUILD_DIR)/src/bootloader.bin
app: $(BUILD_DIR)/src/app.bin

$(BUILD_DIR)/%.$(OBJECT_EXT): %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ -c $<

$(BUILD_DIR)/%.$(OBJECT_EXT): %.s
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ -c $<

$(BUILD_DIR)/src/app.ihx: $(APP_OBJECTS) $(DRIVER_OBJECTS)
	$(CC) $^ $(LDFLAGS) $(CFLAGS) -o $@

$(BUILD_DIR)/src/app.bin: $(BUILD_DIR)/src/app.ihx
	objcopy -Iihex -Obinary $< $@

$(BUILD_DIR)/src/bootloader.ihx: $(BOOTLOADER_OBJECTS) $(DRIVER_CORE_OBJECTS)
	$(CC) $^ $(LDFLAGS) $(CFLAGS) -o $@

$(BUILD_DIR)/src/bootloader.bin: $(BUILD_DIR)/src/bootloader.ihx
	objcopy -Iihex -Obinary $< $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: clean 
