#pragma once
#include "bitmap.h"
#include "color.h"
#include "drivers/display_dimensions.h"
#include "font.h"

uint16_t renderer_get_centered_x(uint16_t width);
uint16_t renderer_get_centered_y(uint16_t height);

void renderer_fill_screen(color_t color);

void renderer_draw_indexed_bitmap(uint16_t x, uint16_t y,
                                  const indexed_bitmap_t *bitmap,
                                  const color_palette_t palette);
void renderer_draw_rgb565_bitmap(uint16_t x, uint16_t y,
                                 const rgb565_bitmap_t *bitmap);
void renderer_draw_text(uint16_t x, uint16_t y, const char *text,
                        const font_t *font, color_t fg, color_t bg);

void renderer_draw_char(uint16_t x, uint16_t y, const char c,
                        const font_t *font, color_t fg, color_t bg);

void renderer_draw_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                        color_t color);

void renderer_begin_stream(void);
void renderer_write_to_stream(uint16_t *pixels, uint32_t count);
void renderer_end_stream(void);
