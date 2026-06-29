#pragma once
#include "mcu/gpio.h"
#include "mcu/spi.h"
#include <stdint.h>

#if !defined(DISPLAY_DRIVER_ILI9341) && !defined(DISPLAY_DRIVER_ST7796)
#error "Please set DISPLAY_DRIVER var in the makefile"
#endif

/*
  ILI9341 and ST7796 LCD drivers both share essentially the same commands.
  Just need to swap between pixel height/width values and MADCTL values.
*/

#if defined(DISPLAY_DRIVER_ILI9341)
#define DISPLAY_HEIGHT_PIXELS (uint16_t)240
#define DISPLAY_WIDTH_PIXELS (uint16_t)320
#elif defined(DISPLAY_DRIVER_ST7796)
#define DISPLAY_HEIGHT_PIXELS (uint16_t)480
#define DISPLAY_WIDTH_PIXELS (uint16_t)320
#endif

typedef struct {
  spi_t *spi;

  gpio_pin_t cs;
  gpio_pin_t dc;
  gpio_pin_t rst;
  gpio_pin_t bl;
} display_config_t;

void display_init(display_config_t *config);
void display_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

void display_pixel_stream_begin(void);
// NOTE: pixel_stream is assumed to be in big endian
void display_pixel_stream_write(const uint16_t *pixels, uint16_t count);
void display_pixel_stream_end(void);
