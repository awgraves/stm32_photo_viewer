#pragma once
#include "registers.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
  GPIO_t *port;
  uint8_t pinno;
} gpio_pin_t;

// RM0390 pg. 185
typedef enum {
  GPIO_MODE_INPUT,
  GPIO_MODE_OUTPUT,
  GPIO_MODE_AF,
  GPIO_MODE_ANALOG,
} gpio_mode_t;

// RM0390 pg. 171
static inline void _gpio_enable_clock(GPIO_t *port) {
  if (port == GPIOA) {
    RCC->AHB1ENR |= AHB1ENR_GPIOA;
  }
}

static inline void gpio_set_mode(gpio_pin_t *pin, gpio_mode_t mode) {
  _gpio_enable_clock(pin->port);

  pin->port->MODER &= ~(3U << (2 * pin->pinno));
  pin->port->MODER |= (mode << (2 * pin->pinno));
}

static inline void gpio_set_AF(gpio_pin_t *pin, uint8_t af) {
  if (pin->pinno < 8) {
    pin->port->AFRL &= ~(0xFU << (4 * pin->pinno));
    pin->port->AFRL |= ((af & 0xFU) << (4 * pin->pinno));
  } else {
    pin->port->AFRH &= ~(0xF << (4 * (pin->pinno - 8)));
    pin->port->AFRH |= ((af & 0xFU) << (4 * (pin->pinno - 8)));
  }
}

static inline void gpio_set_pin(gpio_pin_t *pin) {
  pin->port->BSRR = (1U << pin->pinno);
}

static inline void gpio_clear_pin(gpio_pin_t *pin) {
  pin->port->BSRR = (1U << (pin->pinno + 16));
}

static inline void gpio_write(gpio_pin_t *pin, bool val) {
  val ? gpio_set_pin(pin) : gpio_clear_pin(pin);
}
