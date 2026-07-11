#pragma once
#include "graphics/color.h"
#include "graphics/font.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
  uint16_t x, y, height, width;
  const char *text;
  const font_t *font;
  color_t text_color;
  color_t bg_color;
  bool selected;
  bool focused;
} option_row_params_t;

void option_row_draw(option_row_params_t *p);
