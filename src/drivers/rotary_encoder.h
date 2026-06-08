#pragma once
#include "mcu/gpio.h"

typedef struct {
  gpio_pin_t sw1;
  gpio_pin_t enca;
  gpio_pin_t encb;
} rotary_encoder_config_t;

void rotary_encoder_init(rotary_encoder_config_t *config);
void rotary_encoder_button_poll(void);
