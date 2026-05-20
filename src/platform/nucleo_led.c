#include "nucleo_led.h"
#include "stm32f446re/gpio.h"

#define LED_PORT GPIOA
#define LED_PIN 5

void led_init(void) {
  GPIO_enable_clock(LED_PORT);
  GPIO_set_pin_mode(LED_PORT, LED_PIN, GPIO_MODE_OUTPUT);
}

void led_on(void) { GPIO_pin_set(LED_PORT, LED_PIN); }
