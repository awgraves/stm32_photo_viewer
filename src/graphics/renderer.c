#include "renderer.h"
#include "drivers/ili9341.h"

#define BUFF_SIZE (ILI9341_WIDTH_PIXELS * 20)
static uint16_t buff[BUFF_SIZE];
static uint32_t buff_idx = 0;

void renderer_fill_screen(color_t color) {
  for (buff_idx = 0; buff_idx < BUFF_SIZE; buff_idx++) {
    buff[buff_idx] = color;
  }

  ili9341_set_window(0, 0, ILI9341_WIDTH_PIXELS - 1, ILI9341_HEIGHT_PIXELS - 1);

  ili9341_pixel_stream_begin();
  for (int i = 0; i <= ILI9341_PIXEL_COUNT; i += BUFF_SIZE) {
    ili9341_pixel_stream_write(buff, BUFF_SIZE);
  }
}

static inline uint16_t get_indexed_pixel(const indexed_bitmap_t *bm,
                                         uint32_t pos,
                                         const color_palette_t palette) {
  uint32_t byte = pos >> 3;
  uint32_t bit = pos & 0x7;

  uint8_t pixel_index = (bm->pixels[byte] >> (7 - bit)) & 0x1;

  return palette[pixel_index];
}

void renderer_draw_indexed_bitmap(uint16_t x, uint16_t y,
                                  const indexed_bitmap_t *bitmap,
                                  const color_palette_t palette) {

  ili9341_set_window(x, y, x + bitmap->px_width - 1, y + bitmap->px_height - 1);

  buff_idx = 0;
  uint32_t count = bitmap->px_height * bitmap->px_width;
  uint32_t pos = 0;
  ili9341_pixel_stream_begin();
  // loop through each byte and each bit, convert each to color bitmap
  while (pos < count) {
    buff[buff_idx++] = get_indexed_pixel(bitmap, pos, palette);
    if (buff_idx == BUFF_SIZE) {
      ili9341_pixel_stream_write(buff, buff_idx);
      buff_idx = 0;
    }
    pos++;
  }
  // flush any left over
  if (buff_idx > 0) {
    ili9341_pixel_stream_write(buff, buff_idx);
  }

  ili9341_pixel_stream_end();
}
