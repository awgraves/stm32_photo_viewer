#
# Vars
#
CC=arm-none-eabi-gcc
GDB=arm-none-eabi-gdb
OBJ_SIZE=arm-none-eabi-size

LS_PATH=./src/platform/link.ld
BUILD_DIR=./build
OBJ_DIR=$(BUILD_DIR)/objs

CFLAGS=-c -mcpu=cortex-m4 -mthumb -std=gnu11 \
				 -g -Wall -Werror

LDFLAGS=-mcpu=cortex-m4 -mthumb \
				-nostdlib \
				-T $(LS_PATH) -Wl,-Map=$(BUILD_DIR)/code.map

#
# Files to Process
#
SOURCES = platform/startup.c \
					platform/time.c \
					platform/gpio.c \
					drivers/led.c \
					main.c

OBJ_NAMES = $(SOURCES:.c=.o)
OBJECTS = $(addprefix $(OBJ_DIR)/,$(OBJ_NAMES))

#
# Targets
#
build: $(BUILD_DIR)/code.elf
	@$(OBJ_SIZE) $(BUILD_DIR)/code.elf

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
	$(GDB) $(BUILD_DIR)/code.elf -ex "target remote :3333"
