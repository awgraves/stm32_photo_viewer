#pragma once
#include "mcu/gpio.h"
#include "mcu/spi.h"
#include <stdint.h>

#define ILI9341_HEIGHT_PIXELS (uint16_t)240
#define ILI9341_WIDTH_PIXELS (uint16_t)320
#define ILI9341_PIXEL_COUNT (ILI9341_WIDTH_PIXELS * ILI9341_HEIGHT_PIXELS)

typedef struct {
  spi_t *spi;

  gpio_pin_t cs;
  gpio_pin_t dc;
  gpio_pin_t rst;
} ili9341_config_t;

void ili9341_init(ili9341_config_t *config);
void ili9341_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

void ili9341_pixel_stream_begin(void);
// NOTE: pixel_stream is assumed to be in big endian
void ili9341_pixel_stream_write(uint16_t *pixels, uint16_t count);
void ili9341_pixel_stream_end(void);
