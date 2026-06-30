#include "renderer.h"
#include "drivers/display.h"

#define DISPLAY_PIXELS_TOTAL (DISPLAY_HEIGHT_PIXELS * DISPLAY_WIDTH_PIXELS)

#define BUFF_SIZE (10 * 1024 / 2) // 10kb out of 128kb total RAM
static uint16_t buff[BUFF_SIZE];
static uint32_t buff_idx = 0;

static inline void stream_begin(void);
static inline void stream_pixel(uint16_t pixel);
static inline void stream_end(void);

static inline uint16_t get_1bpp_pixel(const uint8_t *bytes, uint32_t pos,
                                      const color_palette_t palette);
static void draw_1bpp_pixels(uint32_t pixel_count, const uint8_t *bytes,
                             const color_palette_t palette);

/*
Public API
*/

uint16_t renderer_get_centered_x(uint16_t width) {
  return (DISPLAY_WIDTH_PIXELS / 2) - (width / 2) - 1;
}
uint16_t renderer_get_centered_y(uint16_t height) {
  return (DISPLAY_HEIGHT_PIXELS / 2) - (height / 2) - 1;
}

void renderer_fill_screen(color_t color) {
  for (buff_idx = 0; buff_idx < BUFF_SIZE; buff_idx++) {
    buff[buff_idx] = color;
  }

  uint32_t remaining = DISPLAY_PIXELS_TOTAL;

  display_set_window(0, 0, DISPLAY_WIDTH_PIXELS - 1, DISPLAY_HEIGHT_PIXELS - 1);

  display_pixel_stream_begin();

  uint32_t chunk;
  while (remaining) {
    chunk = (remaining > BUFF_SIZE) ? BUFF_SIZE : remaining;

    display_pixel_stream_write(buff, chunk);

    remaining -= chunk;
  }

  display_pixel_stream_end();
}

void renderer_draw_indexed_bitmap(uint16_t x, uint16_t y,
                                  const indexed_bitmap_t *bitmap,
                                  const color_palette_t palette) {

  display_set_window(x, y, x + bitmap->width_px - 1, y + bitmap->height_px - 1);

  uint32_t total_pixels = bitmap->height_px * bitmap->width_px;
  draw_1bpp_pixels(total_pixels, bitmap->pixels, palette);
}

void renderer_draw_rgb565_bitmap(uint16_t x, uint16_t y,
                                 const rgb565_bitmap_t *bitmap) {

  display_set_window(x, y, x + bitmap->width_px - 1, y + bitmap->height_px - 1);

  display_pixel_stream_begin();
  display_pixel_stream_write(bitmap->pixels,
                             bitmap->height_px * bitmap->width_px);
  display_pixel_stream_end();
}

void renderer_draw_char(uint16_t x, uint16_t y, const char c,
                        const font_t *font, color_t fg, color_t bg) {
  color_palette_t p = {bg, fg};

  const uint8_t *bm;
  uint32_t bytes_per_row = (font->width_px) >> 3;
  uint32_t bytes_per_glyph = font->height_px * bytes_per_row;
  uint32_t pix_count = font->height_px * font->width_px;

  display_set_window(x, y, (x + font->width_px - 1), (y + font->height_px - 1));

  bm = &font->bitmaps[(c - ' ') * bytes_per_glyph];
  draw_1bpp_pixels(pix_count, bm, p);
}

void renderer_draw_text(uint16_t x, uint16_t y, const char *text,
                        const font_t *font, color_t fg, color_t bg) {

  uint16_t runningX = x;
  uint16_t runningY = y;

  char c;
  while ((c = *text++)) {
    if (runningY >= DISPLAY_HEIGHT_PIXELS)
      return; // prevent screen overflow
    renderer_draw_char(runningX, runningY, c, font, fg, bg);

    runningX += font->width_px;
    if (runningX >= DISPLAY_WIDTH_PIXELS) {
      runningX = x;
      runningY += font->height_px;
    }
  }
}

void renderer_draw_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                        color_t color) {
  display_set_window(x, y, (x + width - 1), (y + height - 1));

  uint32_t area = width * height;

  stream_begin();
  while (area-- > 0) {
    stream_pixel(color);
  }
  stream_end();
}

/*
Helpers
*/

static inline void stream_begin(void) {
  buff_idx = 0;
  display_pixel_stream_begin();
}

static inline void stream_pixel(uint16_t pixel) {
  buff[buff_idx++] = pixel;

  if (buff_idx == BUFF_SIZE) {
    display_pixel_stream_write(buff, buff_idx);
    buff_idx = 0;
  }
}

static inline void stream_end(void) {
  // flush any left over
  if (buff_idx > 0) {
    display_pixel_stream_write(buff, buff_idx);
  }

  display_pixel_stream_end();
}

static inline uint16_t get_1bpp_pixel(const uint8_t *bytes, uint32_t pos,
                                      const color_palette_t palette) {
  uint32_t byte = pos >> 3;
  uint32_t bit = pos & 0x7;

  uint8_t pixel_index = (bytes[byte] >> (7 - bit)) & 0x1;

  return palette[pixel_index];
}

static void draw_1bpp_pixels(uint32_t pixel_count, const uint8_t *bitmap,
                             const color_palette_t palette) {
  stream_begin();

  for (uint32_t pos = 0; pos < pixel_count; pos++) {
    stream_pixel(get_1bpp_pixel(bitmap, pos, palette));
  }

  stream_end();
}
