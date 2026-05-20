#pragma once

#include "registers.h"

// RM0390 pg. 171
static inline void GPIO_enable_clock(GPIO_T *port) {
  if (port == GPIOA) {
    RCC->AHB1ENR |= BIT(0);
  }
}

// RM0390 pg. 185
typedef enum {
  GPIO_MODE_INPUT,
  GPIO_MODE_OUTPUT,
  GPIO_MODE_AF,
  GPIO_MODE_ANALOG,
} GPIO_MODE;

static inline void GPIO_set_pin_mode(GPIO_T *port, uint8_t pin,
                                     GPIO_MODE mode) {
  port->MODER &= ~(3U << (2 * pin));
  port->MODER |= (mode << (2 * pin));
}

static inline void GPIO_pin_set(GPIO_T *port, uint8_t pin) {
  port->BSRR = (1U << pin);
}

static inline void GPIO_pin_clear(GPIO_T *port, uint8_t pin) {
  port->BSRR = (1U << (pin + 16));
}
