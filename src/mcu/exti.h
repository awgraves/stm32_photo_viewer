#pragma once
#include "gpio.h"

typedef enum {
  EXTI_TRIGGER_EDGE_RISING,
  EXTI_TRIGGER_EDGE_FALLING
} exti_trigger_edge_t;

typedef void (*exti_callback_t)(void);

typedef struct {
  gpio_pin_t pin;
  exti_trigger_edge_t trigger;
  exti_callback_t callback;
} exti_config_t;

void exti_configure(exti_config_t *config);
