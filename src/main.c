#include "board/board.h"
#include "drivers/ili9341.h"
#include "input/event_queue.h"
#include "mcu/spi.h"
#include "mcu/sysclock.h"
#include "mcu/time.h"
#include "screens/menu.h"
#include "screens/splash.h"

void process_loop(void) {
  input_event_t event;
  while (event_queue_pop(&event)) {
    menu_handle_event(event);
    delay_ms(300);
  }
}

int main() {

  sysclock_init(CPU_FREQ_40_MHZ);
  time_init();

  spi_config_t spi1_conf = {
      .mosi = LCD_SPI_MOSI, .sck = LCD_SPI_SCK, .baud = SPI_BAUD_DIV_2};
  spi_init(&spi1, &spi1_conf);

  ili9341_config_t lcd_c = {
      .spi = &spi1, .cs = LCD_CS, .dc = LCD_DC, .rst = LCD_RST};
  ili9341_init(&lcd_c);

  splash_show();
  menu_show();

  int8_t idx = 0;

  while (1) {
    // do nothing
    while (++idx <= 8) {
      event_queue_push(INPUT_EVENT_ENCODER_CW);
    }

    process_loop();

    while (--idx >= 0) {
      event_queue_push(INPUT_EVENT_ENCODER_CCW);
    }

    process_loop();
  }
}
