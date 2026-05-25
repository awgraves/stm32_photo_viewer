#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef enum {
  PA0 = 0x00,
  PA1 = 0X01,
  PA2 = 0x02,
  PA3 = 0x03,
  PA4 = 0x04,
  PA5 = 0x05,
  PA6 = 0x06,
  PA7 = 0x07,
  PA8 = 0x08,
  PA9 = 0x09,
  PA10 = 0x0A,
  PA11 = 0x0B,
  PA12 = 0x0C,
  PA13 = 0x0D,
  PA14 = 0x0E,
  PA15 = 0x0F
} gpio_pin_t;

// RM0390 pg. 185
typedef enum {
  GPIO_MODE_INPUT,
  GPIO_MODE_OUTPUT,
  GPIO_MODE_AF,
  GPIO_MODE_ANALOG,
} gpio_mode_t;

// RM0390 pg. 58
typedef enum {
  GPIO_AF_SPI1 = 0x5,
} gpio_af_t;

void gpio_set_mode(gpio_pin_t pin, gpio_mode_t mode);
void gpio_set_AF(gpio_pin_t pin, gpio_af_t af);

void gpio_set_pin(gpio_pin_t pin);
void gpio_clear_pin(gpio_pin_t pin);
void gpio_write(gpio_pin_t pin, bool val);
