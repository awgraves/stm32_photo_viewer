#include "led.h"
#include "../board/board.h"
#include "../mcu/gpio.h"

void led_init(void) { gpio_set_mode(LED_PIN, GPIO_MODE_OUTPUT); }

void led_on(void) { gpio_set_pin(LED_PIN); }
void led_off(void) { gpio_clear_pin(LED_PIN); }
