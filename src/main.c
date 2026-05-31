#include "assets/logo.h"
#include "board/board.h"
#include "drivers/ili9341.h"
#include "graphics/renderer.h"
#include "mcu/spi.h"
#include "mcu/sysclock.h"
#include "mcu/time.h"

int main() {

  sysclock_init(CPU_FREQ_40_MHZ);
  time_init();

  spi_config_t spi1_conf = {
      .mosi = LCD_SPI_MOSI, .sck = LCD_SPI_SCK, .baud = SPI_BAUD_DIV_2};
  spi_init(&spi1, &spi1_conf);

  ili9341_config_t lcd_c = {
      .spi = &spi1, .cs = LCD_CS, .dc = LCD_DC, .rst = LCD_RST};
  ili9341_init(&lcd_c);

  renderer_fill_screen(COLOR_GREEN);
  color_palette_t p = {COLOR_BLACK, COLOR_GREEN};
  renderer_draw_indexed_bitmap(110, 70, &logo, p);

  while (1) {
    // do nothing
  }
}
