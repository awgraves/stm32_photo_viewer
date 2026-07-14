#pragma once
#include "graphics/bitmap.h"
#include "graphics/color.h"
#include "graphics/font.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
  uint16_t x, y, height, width;
  const indexed_bitmap_t *icon;
  const font_t *font;
  color_t primary_color;   // bar color when not focused
  color_t secondary_color; // bg color when not focused
  uint8_t brightness_val;
  bool focused;
} brightness_bar_params_t;

// the full draw
void brightness_bar_draw(brightness_bar_params_t *p);

// targeted draw for smoother updates
void brightness_bar_draw_values_only(brightness_bar_params_t *p);
