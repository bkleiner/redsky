CC = sdcc

OBJECT_EXT = rel

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