#pragma once
#include "bitmap.h"
#include "color.h"
#include "font.h"

void renderer_fill_screen(color_t color);
void renderer_draw_indexed_bitmap(uint16_t x, uint16_t y,
                                  const indexed_bitmap_t *bitmap,
                                  const color_palette_t palette);
void renderer_draw_rgb565_bitmap(uint16_t x, uint16_t y,
                                 const rgb565_bitmap_t *bitmap);
void renderer_draw_text(uint16_t x, uint16_t y, const char *text,
                        const font_t *font, color_t fg, color_t bg);
