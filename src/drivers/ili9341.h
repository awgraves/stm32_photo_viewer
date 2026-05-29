#pragma once
#include "mcu/gpio.h"
#include "mcu/spi.h"
#include <stdint.h>

#define ILI9341_HEIGHT_PIXELS 240
#define ILI9341_WIDTH_PIXELS 320

typedef struct {
  spi_t *spi;

  gpio_pin_t cs;
  gpio_pin_t dc;
  gpio_pin_t rst;
} ili9341_config_t;

void ili9341_init(ili9341_config_t *config);
void ili9341_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void ili9341_write_pixels(uint16_t *pixels, uint16_t count);
