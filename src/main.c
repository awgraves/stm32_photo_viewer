#include "board/board.h"
#include "drivers/ili9341.h"
#include "drivers/rotary_encoder.h"
#include "input/event_queue.h"
#include "mcu/spi.h"
#include "mcu/sysclock.h"
#include "mcu/time.h"
#include "mcu/timer.h"
#include "screens/menu.h"
// #include "screens/splash.h"

void process_loop(void) {
  input_event_t event;
  while (event_queue_pop(&event)) {
    menu_handle_event(event);
    // delay_ms(300);
  }
}

#define POLL_MS_INTERVAL 10
static uint32_t last_poll = 0;
static uint32_t now = 0;

int main() {

  sysclock_init(CPU_FREQ_40_MHZ);
  time_init();

  spi_config_t spi1_conf = {
      .mosi = LCD_SPI_MOSI, .sck = LCD_SPI_SCK, .baud = SPI_BAUD_DIV_2};
  spi_init(&spi1, &spi1_conf);

  ili9341_config_t lcd_conf = {
      .spi = &spi1, .cs = LCD_CS, .dc = LCD_DC, .rst = LCD_RST};
  ili9341_init(&lcd_conf);

  rotary_encoder_config_t rot_conf = {.sw1 = ENC_CENTER,
                                      .enca = ENC_A,
                                      .encb = ENC_B,
                                      .enc_af = GPIO_AF_TIM2,
                                      .timer = &timer2};
  rotary_encoder_init(&rot_conf);

  // splash_show();
  menu_show();

  while (1) {
    now = millis();
    if (now - last_poll > POLL_MS_INTERVAL) {
      rotary_encoder_poll();
      last_poll = now;
    }
    process_loop();
  }
}
