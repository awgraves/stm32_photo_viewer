#pragma once
#include "mcu/gpio.h"
#include "mcu/spi.h"
#include "mcu/timer.h"
#include <stdint.h>

typedef struct {
  gpio_pin_t cs;
  gpio_pin_t dc;
  gpio_pin_t rst;
  gpio_pin_t bl;

  spi_t *spi;

  timer_t *timer;
  gpio_af_t bl_af;
  uint8_t brightness_val; // must be 0 - 100
} display_config_t;

void display_init(display_config_t *config);
void display_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

void display_pixel_stream_begin(void);
// NOTE: pixel_stream is assumed to be in big endian
void display_pixel_stream_write(const uint16_t *pixels, uint32_t count);
void display_pixel_stream_end(void);

// brightness val is between 0 and 100
uint8_t display_get_brightness(void);
// brightness val is between 0 and 100
void display_set_brightness(uint8_t val);
