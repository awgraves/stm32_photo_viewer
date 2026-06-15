#pragma once
#include "mcu/gpio.h"
#include "mcu/timer.h"
#include <stdbool.h>

typedef struct {
  gpio_pin_t sw1; // center button
  gpio_pin_t enca;
  gpio_pin_t encb;

  gpio_af_t enc_af; // for enca/encb
  timer_t *timer;
} rotary_encoder_config_t;

typedef struct {
  int16_t delta;
  bool button_pressed;
} rotary_state_t;

void rotary_encoder_init(rotary_encoder_config_t *config);
rotary_state_t rotary_encoder_get_state(void);
