#pragma once
#include <stdint.h>

/*
ili9341 driver casts these as uint8[] because of 8-bit spi.
due to arm being little endian, 8F00 becomes 00,8F over the wire
so swapping them preemptively saves performance
*/
#define RGB565(x) ((x >> 8) | (x << 8))

typedef enum {
  COLOR_RED = RGB565(0x8F00U),
  COLOR_GREEN = RGB565(0x07E0U),
  COLOR_BLUE = RGB565(0x001FU),
  COLOR_BLACK = RGB565(0x0000U),
  COLOR_WHITE = RGB565(0xFFFFU),
} color_t;

typedef color_t color_palette_t[];
