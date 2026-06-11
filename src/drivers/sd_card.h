#pragma once
#include "mcu/gpio.h"
#include <stdbool.h>

typedef struct {
  gpio_pin_t det; // detect pin
} sd_card_config_t;

void sd_card_init(sd_card_config_t *config);
bool sd_card_check_inserted(void);
