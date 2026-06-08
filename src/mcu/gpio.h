#pragma once
#include <stdint.h>

#define PINNO(enum_val) (enum_val & 0xFU)

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

typedef enum { LOW, HIGH } gpio_digital_t;

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

typedef enum {
  GPIO_SPEED_SLOW,
  GPIO_SPEED_MEDIUM,
  GPIO_SPEED_FAST,
  GPIO_SPEED_HIGH
} gpio_speed_t;

// RM0390 pg. 186
typedef enum {
  GPIO_PUPD_NONE,
  GPIO_PUPD_PULL_UP,
  GPIO_PUPD_PULL_DOWN,
} gpio_pupdr_t;

void gpio_set_mode(gpio_pin_t pin, gpio_mode_t mode);
void gpio_set_AF(gpio_pin_t pin, gpio_af_t af);
void gpio_set_ospeed(gpio_pin_t pin, gpio_speed_t speed);
void gpio_set_pupd(gpio_pin_t pin, gpio_pupdr_t pupdr);

void gpio_set_pin(gpio_pin_t pin);
void gpio_clear_pin(gpio_pin_t pin);
void gpio_digital_write(gpio_pin_t pin, gpio_digital_t val);

gpio_digital_t gpio_digital_read(gpio_pin_t pin);
