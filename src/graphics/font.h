#pragma once
#include "stdint.h"

#define LOCHAR 32
#define HICHAR 127

typedef struct {
  uint8_t height;
  uint8_t width;
  const char *bitmaps;
} font_t;

extern const font_t kourier_16;
