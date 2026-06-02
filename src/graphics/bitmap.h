#pragma once
#include <stdint.h>

typedef struct {
  uint16_t height_px;
  uint16_t width_px;

  const uint8_t *pixels;
} indexed_bitmap_t;

typedef struct {
  uint16_t height_px;
  uint16_t width_px;

  const uint16_t *pixels;
} rgb565_bitmap_t;
