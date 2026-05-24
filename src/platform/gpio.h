#pragma once
#include <stdbool.h>
#include <stdint.h>

#define PIN(port, num) ((((port) - 'A') << 8) | (num))
#define PINNO(pin) (pin & 255)
#define PINPORT(pin) (pin >> 8)

typedef uint16_t pin_t;

// RM0390 pg. 185
typedef enum {
  GPIO_MODE_INPUT,
  GPIO_MODE_OUTPUT,
  GPIO_MODE_AF,
  GPIO_MODE_ANALOG,
} gpio_mode_t;

typedef enum { GPIO_AF_SPI1 } gpio_af_t;

void gpio_set_mode(pin_t pin, gpio_mode_t mode);
void gpio_set_AF(pin_t pin, gpio_af_t af);

void gpio_write(pin_t pin, bool val);
void gpio_set_pin(pin_t pin);
void gpio_clear_pin(pin_t pin);
