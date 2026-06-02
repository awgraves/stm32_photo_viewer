#pragma once
#include "stdint.h"

#define LOCHAR 32
#define HICHAR 127

typedef struct {
  uint8_t height_px;
  uint8_t width_px;
  const uint8_t *bitmaps;
} font_t;

extern const font_t ibm_bios_16;
extern const font_t terminus_bold_16;
