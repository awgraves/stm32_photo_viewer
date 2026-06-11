#pragma once
#include "gpio.h"

typedef struct {
  gpio_pin_t clk;
  gpio_pin_t cmd;
  gpio_pin_t det;
  gpio_pin_t d3;
  gpio_pin_t d2;
  gpio_pin_t d1;
  gpio_pin_t d0;
} sdio_config_t;

void sdio_init(sdio_config_t *config);
