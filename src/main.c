#include "board/board.h"
#include "drivers/ili9341.h"
#include "mcu/spi.h"
#include "mcu/sysclock.h"
#include "mcu/time.h"
#include "screens/menu.h"
#include "screens/splash.h"

int main() {

  sysclock_init(CPU_FREQ_40_MHZ);
  time_init();

  spi_config_t spi1_conf = {
      .mosi = LCD_SPI_MOSI, .sck = LCD_SPI_SCK, .baud = SPI_BAUD_DIV_2};
  spi_init(&spi1, &spi1_conf);

  ili9341_config_t lcd_c = {
      .spi = &spi1, .cs = LCD_CS, .dc = LCD_DC, .rst = LCD_RST};
  ili9341_init(&lcd_c);

  screens_splash_show();
  screens_menu_show();

  while (1) {
    // do nothing
  }
}
