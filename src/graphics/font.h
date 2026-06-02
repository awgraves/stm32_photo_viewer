#pragma once
#include "stdint.h"

typedef struct {
  uint8_t height_px;
  uint8_t width_px;
  const uint8_t *bitmaps;
} font_t;
