#pragma once
#include <stdint.h>

/*
ili9341 driver casts these as uint8[] because of 8-bit spi.
due to arm being little endian, 8F00 becomes 00,8F over the wire
so swapping them preemptively saves performance
*/
#define RGB565(x) (uint16_t)((x >> 8) | (x << 8))

typedef enum {
  COLOR_RED = RGB565(0x8F00U),
  COLOR_GREEN = RGB565(0x07E0U),
  COLOR_SPLASH_BLACK = RGB565(0x18E3),
  COLOR_SPLASH_GREY = RGB565(0xA534),
  COLOR_BLUE = RGB565(0x001FU),
  COLOR_BLUE_ALT = RGB565(0x001AU),
  COLOR_BLACK = RGB565(0x0000U),
  COLOR_WHITE = RGB565(0xFFFFU),
} color_t;

typedef uint16_t color_palette_t[];
