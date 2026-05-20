#include "platform/nucleo_led.h"

int main() {
  led_init();

  while (1) {
    for (int i = 0; i < 500000; i++)
      ;
    led_on();
    for (int i = 0; i < 500000; i++)
      ;
    led_off();
  }
}
