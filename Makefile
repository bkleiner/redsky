BUILD_DIR := build
TARGET    ?= d8r
MODE 			?= release

BOOTLOADER_SIZE := 0x2000

include board/$(TARGET)/board.mk
include $(DRIVER_DIR)/driver.mk

INCLUDE_DIRS := $(BOARD_DIR) \
								$(DRIVER_INCLUDE_DIRS) \
								src/app \
								src/config

CFLAGS += -DFLASH_SIZE=$(FLASH_SIZE) \
					-DBOOTLOADER_SIZE=$(BOOTLOADER_SIZE) \
					$(addprefix -I ,$(INCLUDE_DIRS)) 

BOOTLOADER_SOURCES += src/bootloader/bootloader.c
BLINKY_SOURCES += src/blinky/blinky.c
APP_SOURCES += src/app/app.c \
							 src/app/redpine.c \
							 src/app/debug.c

DRIVER_CORE_OBJECTS=$(addsuffix .$(OBJECT_EXT),$(addprefix $(BUILD_DIR)/,$(basename $(DRIVER_CORE_SOURCES))))
DRIVER_OBJECTS=$(addsuffix .$(OBJECT_EXT),$(addprefix $(BUILD_DIR)/,$(basename $(DRIVER_SOURCES))))

BOOTLOADER_OBJECTS=$(addsuffix .$(OBJECT_EXT),$(addprefix $(BUILD_DIR)/,$(basename $(BOOTLOADER_SOURCES))))
BLINKY_OBJECTS=$(addsuffix .$(OBJECT_EXT),$(addprefix $(BUILD_DIR)/,$(basename $(BLINKY_SOURCES))))
APP_OBJECTS=$(addsuffix .$(OBJECT_EXT),$(addprefix $(BUILD_DIR)/,$(basename $(APP_SOURCES))))

all: bootloader blinky app

bootloader: $(BUILD_DIR)/src/bootloader.bin
blinky: $(BUILD_DIR)/src/blinky.bin
app: $(BUILD_DIR)/src/app.bin

$(BUILD_DIR)/%.$(OBJECT_EXT): %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ -c $<

$(BUILD_DIR)/%.$(OBJECT_EXT): %.s
	@mkdir -p $(@D)
	$(AS) $(ASFLAGS) -o $@ -c $<

$(BUILD_DIR)/%.$(OBJECT_EXT): %.asm
	@mkdir -p $(@D)
	$(AS) $(ASFLAGS) $@ $<

$(BUILD_DIR)/src/app.$(TARGET_EXT): $(APP_OBJECTS) $(DRIVER_OBJECTS)
	$(CC) $^ $(APP_LDFLAGS) $(CFLAGS) -o $@

$(BUILD_DIR)/src/app.bin: $(BUILD_DIR)/src/app.$(TARGET_EXT)
	$(CP) $(CP_FLAGS) -Obinary $< $@

$(BUILD_DIR)/src/bootloader.$(TARGET_EXT): $(BOOTLOADER_OBJECTS) $(DRIVER_CORE_OBJECTS)
	$(CC) $^ $(BOOTLOADER_LDFLAGS) $(CFLAGS) -o $@

$(BUILD_DIR)/src/bootloader.bin: $(BUILD_DIR)/src/bootloader.$(TARGET_EXT)
	$(CP) $(CP_FLAGS) -Obinary $< $@

$(BUILD_DIR)/src/blinky.$(TARGET_EXT): $(BLINKY_OBJECTS) $(DRIVER_CORE_OBJECTS)
	$(CC) $^ $(APP_LDFLAGS) $(CFLAGS) -o $@

$(BUILD_DIR)/src/blinky.bin: $(BUILD_DIR)/src/blinky.$(TARGET_EXT)
	$(CP) $(CP_FLAGS) -Obinary $< $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: clean 
