#include "led.h"
#include "../platform/gpio.h"

#define LED_PIN PIN('A', 5)

void led_init(void) { gpio_set_mode(LED_PIN, GPIO_MODE_OUTPUT); }

void led_on(void) { gpio_set_pin(LED_PIN); }
void led_off(void) { gpio_clear_pin(LED_PIN); }
