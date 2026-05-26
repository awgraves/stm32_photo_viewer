#include "board/board.h"
#include "mcu/spi.h"
#include "mcu/time.h"

int main() {
  time_init();

  spi_config_t spi1_conf = {
      .mosi = LCD_SPI_MOSI, .sck = LCD_SPI_SCK, .baud = SPI_BAUD_DIV_2};

  spi_init(&spi1, &spi1_conf);
  gpio_set_mode(LCD_CS, GPIO_MODE_OUTPUT);

  uint8_t data[3] = {0xAA, 0xFF, 0x55};

  while (1) {
    gpio_set_pin(LCD_CS);
    spi_tx(&spi1, data, 3);
    gpio_clear_pin(LCD_CS);
    delay_ms(1000);
  }
}
