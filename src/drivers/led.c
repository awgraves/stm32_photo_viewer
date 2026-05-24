#include "led.h"
#include "../board/board.h"

void led_init(void) { gpio_set_mode(&led_pin, GPIO_MODE_OUTPUT); }

void led_on(void) { gpio_set_pin(&led_pin); }
void led_off(void) { gpio_clear_pin(&led_pin); }
