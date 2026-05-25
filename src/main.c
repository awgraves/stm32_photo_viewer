#include "drivers/led.h"
#include "mcu/time.h"

int main() {
  time_init();
  led_init();

  while (1) {
    delay_ms(100);
    led_on();
    delay_ms(100);
    led_off();
  }
}
