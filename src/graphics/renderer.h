#pragma once
#include "bitmap.h"
#include "color.h"

void renderer_fill_screen(color_t color);
void renderer_draw_indexed_bitmap(uint16_t x, uint16_t y,
                                  const indexed_bitmap_t *bitmap,
                                  const color_palette_t palette);
