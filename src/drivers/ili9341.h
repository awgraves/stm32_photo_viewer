#pragma once
#include "mcu/gpio.h"
#include "mcu/spi.h"
#include <stdint.h>

#define ILI9341_HEIGHT 240
#define ILI9341_WIDTH 320

typedef enum {
  COLOR_RED = 0xF800,
  COLOR_GREEN = 0x0FF0,
  COLOR_BLUE = 0x008F
} color_t;

typedef struct {
  spi_t *spi;

  gpio_pin_t cs;
  gpio_pin_t dc;
  gpio_pin_t rst;
} ili9341_config_t;

void ili9341_init(ili9341_config_t *config);
void ili9341_fill(uint16_t color);
