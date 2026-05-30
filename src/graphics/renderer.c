#include "renderer.h"
#include "drivers/ili9341.h"

#define BUFF_SIZE (ILI9341_WIDTH_PIXELS * 20)
uint16_t buff[BUFF_SIZE];

void renderer_fill_screen(color_t color) {
  for (int i = 0; i < BUFF_SIZE; i++) {
    buff[i] = rgb565_swap(color);
  }

  ili9341_set_window(0, 0, ILI9341_WIDTH_PIXELS - 1, ILI9341_HEIGHT_PIXELS - 1);

  ili9341_pixel_stream_begin();
  for (int i = 0; i <= ILI9341_PIXEL_COUNT; i += BUFF_SIZE) {
    ili9341_pixel_stream_write(buff, BUFF_SIZE);
  }
  ili9341_pixel_stream_end();
}
