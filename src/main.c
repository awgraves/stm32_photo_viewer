#include "board/board.h"
#include "drivers/ili9341.h"
#include "mcu/spi.h"
#include "mcu/sysclock.h"
#include "mcu/time.h"

int main() {

  sysclock_init(CPU_FREQ_32_MHZ);
  time_init();

  spi_config_t spi1_conf = {
      .mosi = LCD_SPI_MOSI, .sck = LCD_SPI_SCK, .baud = SPI_BAUD_DIV_2};
  spi_init(&spi1, &spi1_conf);

  ili9341_config_t lcd_c = {
      .spi = &spi1, .cs = LCD_CS, .dc = LCD_DC, .rst = LCD_RST};
  ili9341_init(&lcd_c);

  while (1) {
    ili9341_fill(COLOR_RED);
    delay_ms(1000);
    ili9341_fill(COLOR_GREEN);
    delay_ms(1000);
    ili9341_fill(COLOR_BLUE);
    delay_ms(1000);
  }
}
