#pragma once
#include "graphics/color.h"
#include "graphics/font.h"

typedef struct {
  uint16_t x, y, width, height;
  const char *title_text;
  const font_t *title_font;
  color_t fill_color;
  color_t line_color;
  uint8_t line_thickness;
} window_params_t;

void window_draw(window_params_t *p);
