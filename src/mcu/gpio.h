#pragma once
#include <stdint.h>

#define PINNO(enum_val) (enum_val & 0xFU)

/*
   Using an enum bit shift trick here.
   Each single byte enum val holds upper nibble 4 bits to represent port
   and lower nibble 4 bits to represent the pinno within that port.

   This works out nicely for hex increments.
*/
typedef enum {
  // Port A
  PA0 = 0x00U,
  PA1,
  PA2,
  PA3,
  PA4,
  PA5,
  PA6,
  PA7,
  PA8,
  PA9,
  PA10,
  PA11,
  PA12,
  PA13,
  PA14,
  PA15,
  // Port C
  PC0 = 0x20U,
  PC1,
  PC2,
  PC3,
  PC4,
  PC5,
  PC6,
  PC7,
  PC8,
  PC9,
  PC10,
  PC11,
  PC12,
  PC13,
  PC14,
  PC15,
  // Port D
  PD0 = 0x30U,
  PD1,
  PD2,
  PD3,
  PD4,
  PD5,
  PD6,
  PD7,
  PD8,
  PD9,
  PD10,
  PD11,
  PD12,
  PD13,
  PD14,
  PD15,
} gpio_pin_t;

typedef enum { LOW, HIGH } gpio_digital_t;

// RM0390 pg. 185
typedef enum {
  GPIO_MODE_INPUT,
  GPIO_MODE_OUTPUT,
  GPIO_MODE_AF,
  GPIO_MODE_ANALOG,
} gpio_mode_t;

// RM0390 pg. 178
typedef enum {
  GPIO_AF_TIM1_TIM2 = 0x1U,
  GPIO_AF_SPI1 = 0x5U,
  GPIO_AF_SDIO = 0xCU,
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
