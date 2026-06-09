#pragma once
#include "mcu/gpio.h"
#include "mcu/timer.h"

typedef struct {
  gpio_pin_t sw1; // center button
  gpio_pin_t enca;
  gpio_pin_t encb;

  gpio_af_t enc_af; // for enca/encb
  timer_t *timer;
} rotary_encoder_config_t;

void rotary_encoder_init(rotary_encoder_config_t *config);
void rotary_encoder_poll(void);
