#pragma once
#include <stdint.h>

typedef struct {
  uint16_t px_height;
  uint16_t px_width;

  const uint8_t *pixels;
} indexed_bitmap_t;
