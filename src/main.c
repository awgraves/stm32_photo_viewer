#include "drivers/led.h"
#include "platform/time.h"

int main() {
  time_init();
  led_init();

  while (1) {
    delay_ms(500);
    led_on();
    delay_ms(500);
    led_off();
  }
}
