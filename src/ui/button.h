#pragma once
#include "graphics/color.h"
#include "graphics/font.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
  uint16_t x, y, height, width;
  color_t line_color;
  color_t fill_color;
  const char *text;
  const font_t *font;
  bool highlighted;
} button_params_t;

void button_draw(button_params_t *p);
