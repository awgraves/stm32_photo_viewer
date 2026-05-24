#include "gpio.h"
#include "_registers.h"

#define GPIO(port) ((GPIO_t *)(0x40020000 + 0x400 * (port)))

static void _gpio_enable_clock(GPIO_t *port);

void gpio_set_mode(pin_t pin, gpio_mode_t mode) {
  GPIO_t *port = GPIO(PINPORT(pin));
  _gpio_enable_clock(port);

  port->MODER &= ~(3U << (2 * pin));
  port->MODER |= (mode << (2 * pin));
}

void gpio_set_AF(pin_t pin, gpio_af_t af) {
  GPIO_t *port = GPIO(PINPORT(pin));
  uint8_t pinno = PINNO(pin);

  uint8_t AF = 0;
  switch (af) {
  case GPIO_AF_SPI1:
    AF = AF_SPI;
    break;
  default:
    AF = AF_SPI;
  }

  if (pinno < 8) {
    port->AFRL &= ~(0xFU << (4 * pin));
    port->AFRL |= ((AF & 0xFU) << (4 * pin));
  } else {
    port->AFRH &= ~(0xF << (4 * (pin - 8)));
    port->AFRH |= ((AF & 0xFU) << (4 * (pin - 8)));
  }
}

void gpio_set_pin(pin_t pin) {
  GPIO_t *port = GPIO(PINPORT(pin));
  port->BSRR = (1U << PINNO(pin));
}

void gpio_clear_pin(pin_t pin) {
  GPIO_t *port = GPIO(PINPORT(pin));
  port->BSRR = (1U << (pin + 16));
}

void gpio_write(pin_t pin, bool val) {
  val ? gpio_set_pin(pin) : gpio_clear_pin(pin);
}

// RM0390 pg. 171
static void _gpio_enable_clock(GPIO_t *port) {
  if (port == GPIOA) {
    RCC->AHB1ENR |= AHB1ENR_GPIOA;
  }
}
