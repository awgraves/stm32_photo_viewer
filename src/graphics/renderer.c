#include "renderer.h"
#include "drivers/ili9341.h"

#define SCAN_ROW_HEIGHT_PIXELS 20
#define SCAN_ROW_WIDTH_PIXELS ILI9341_WIDTH_PIXELS
#define SCAN_ROWS_PER_SCREEN (ILI9341_HEIGHT_PIXELS / SCAN_ROW_HEIGHT_PIXELS)
#define SCAN_BUFF_PIXELS_LEN (SCAN_ROW_WIDTH_PIXELS * SCAN_ROW_HEIGHT_PIXELS)

uint16_t scanbuff[SCAN_BUFF_PIXELS_LEN];

void renderer_fill_screen(color_t color) {
  for (int i = 0; i < SCAN_BUFF_PIXELS_LEN; i++) {
    scanbuff[i] = rgb565_swap(color);
  }

  uint16_t row_offset;
  for (int i = 0; i < SCAN_ROWS_PER_SCREEN; i++) {
    row_offset = i * SCAN_ROW_HEIGHT_PIXELS;
    ili9341_set_window(0, row_offset, SCAN_ROW_WIDTH_PIXELS,
                       row_offset + SCAN_ROW_HEIGHT_PIXELS - 1);
    ili9341_write_pixels(scanbuff, SCAN_BUFF_PIXELS_LEN);
  }
}
