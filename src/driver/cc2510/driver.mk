CC = sdcc

OBJECT_EXT = rel

DEBUG_FLAGS = --debug \
							--verbose

CFLAGS  = $(DEBUG_FLAGS) \
					--model-small \
					--opt-code-speed

LDFLAGS = --out-fmt-ihx \
					--code-loc 0x0000 \
					--code-size $(FLASH_SIZE) \
					--xram-loc 0xf000 \
					--xram-size 0x7FF \
					--iram-size 0x100

#	/usr/share/sdcc/include 

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
									$(DRIVER_DIR)/uart_dma.c \
									$(DRIVER_DIR)/crc.c

DRIVER_INCLUDE_DIRS := $(DRIVER_DIR)