#
# Vars
#
CC=arm-none-eabi-gcc
GDB=arm-none-eabi-gdb
OBJ_SIZE=arm-none-eabi-size
SPEED_PROFILE ?= FAST

LS_PATH=./src/mcu/link.ld
BUILD_DIR=./build
OBJ_DIR=$(BUILD_DIR)/objs

CFLAGS=-c -mcpu=cortex-m4 -mthumb -std=gnu11 \
			 -Isrc \
			 -g -Wall -Werror

LDFLAGS=-mcpu=cortex-m4 -mthumb \
				-nostdlib \
				-T $(LS_PATH) -Wl,-Map=$(BUILD_DIR)/code.map

#
# Files to Process
#
SOURCES = mcu/startup.c \
					mcu/sysclock.c \
					mcu/systick.c \
					mcu/timer.c \
					mcu/gpio.c \
					mcu/spi.c \
					mcu/sdio.c \
					drivers/time.c \
					drivers/display.c \
					drivers/rotary_encoder.c \
					drivers/sd_card.c \
					board/board.c \
					graphics/renderer.c \
					assets/bitmaps/splash_logo.c \
					assets/bitmaps/title_text.c \
					assets/fonts/ibm_bios_16.c \
					assets/fonts/terminus_bold_16.c \
					events/event_queue.c \
					inputs/poll.c \
					storage/storage.c \
					storage/fat32.c \
					screens/splash.c \
					screens/menu.c \
					screens/init.c \
					screens/viewer.c \
					photo_album/photo_album.c \
					slideshow/slideshow.c \
					utils/string.c \
					ui/window.c \
					ui/button.c \
					ui/option_row.c \
					ui/focus_list.c \
					main.c

ifeq ($(SPEED_PROFILE), FAST)
	CFLAGS += -DSPEED_PROFILE_FAST
else ifeq ($(SPEED_PROFILE), SLOW)
	CFLAGS += -DSPEED_PROFILE_SLOW
else
	$(error Please specify a SPEED_PROFILE)
endif

OBJ_NAMES = $(SOURCES:.c=.o)
OBJECTS = $(addprefix $(OBJ_DIR)/,$(OBJ_NAMES))

#
# Targets
#
build: $(BUILD_DIR)/code.elf
	@$(OBJ_SIZE) $(BUILD_DIR)/code.elf

@phony setup:
	bear -- make -B

$(BUILD_DIR)/code.elf: $(OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rf build/*

flash: build
	openocd -f board/st_nucleo_f4.cfg -c "program $(BUILD_DIR)/code.elf verify reset exit"

map:
	less $(BUILD_DIR)/code.map

debug_server:
	openocd -f board/st_nucleo_f4.cfg

debug:
	$(GDB) $(BUILD_DIR)/code.elf -ex "target remote :3333" -ex "monitor reset halt"

@phony convert:
	uv run ./tools/convert.py
