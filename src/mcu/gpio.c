#include "gpio.h"
#include "registers.h"

#define PORT(enum_val) (GPIO_t *)(((enum_val >> 4) * sizeof(GPIO_t)) + GPIOA)

// RM0390 pg. 171
static inline void _gpio_enable_clock(GPIO_t *port) {
  if (port == GPIOA) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOA;
  }
}

void gpio_set_mode(gpio_pin_t pin, gpio_mode_t mode) {
  GPIO_t *port = PORT(pin);
  _gpio_enable_clock(port);

  port->MODER &= ~(3U << (2 * PINNO(pin)));
  port->MODER |= (mode << (2 * PINNO(pin)));
}

void gpio_set_AF(gpio_pin_t pin, gpio_af_t af) {
  GPIO_t *port = PORT(pin);
  uint8_t pinno = PINNO(pin);

  if (pinno < 8) {
    port->AFRL &= ~(0xFU << (4 * pinno));
    port->AFRL |= ((af & 0xFU) << (4 * pinno));
  } else {
    port->AFRH &= ~(0xF << (4 * (pinno - 8)));
    port->AFRH |= ((af & 0xFU) << (4 * (pinno - 8)));
  }
}

// when running higher CPU freq, sometimes need more aggressive signal change
void gpio_set_ospeed(gpio_pin_t pin, gpio_speed_t speed) {
  GPIO_t *port = PORT(pin);
  uint8_t pinno = PINNO(pin);

  port->OSPEEDER &= ~((0x3) << (2 * pinno));
  port->OSPEEDER |= ((speed & 0x3) << (2 * pinno));
}

void gpio_set_pupd(gpio_pin_t pin, gpio_pupdr_t pupdr) {
  GPIO_t *port = PORT(pin);
  uint8_t pinno = PINNO(pin);

  port->PUPDR &= ~((0x3) << (2 * pinno));
  port->PUPDR |= ((pupdr & 0x3) << (2 * pinno));
}

void gpio_set_pin(gpio_pin_t pin) {
  GPIO_t *port = PORT(pin);
  port->BSRR = (1U << PINNO(pin));
}

void gpio_clear_pin(gpio_pin_t pin) {
  GPIO_t *port = PORT(pin);
  port->BSRR = (1U << (PINNO(pin) + 16));
}

void gpio_digital_write(gpio_pin_t pin, gpio_digital_t val) {
  val ? gpio_set_pin(pin) : gpio_clear_pin(pin);
}

gpio_digital_t gpio_digital_read(gpio_pin_t pin) {
  GPIO_t *port = PORT(pin);
  uint8_t pinno = PINNO(pin);

  return port->IDR & (BIT(pinno)) ? HIGH : LOW;
}
