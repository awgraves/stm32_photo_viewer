#pragma once
#include "mcu/gpio.h"
#include "mcu/sdio.h"
#include <stdbool.h>

typedef struct {
  gpio_pin_t det;
  sdio_config_t sdio;
} sd_card_config_t;

void sd_card_reader_init(sd_card_config_t *config);
bool sd_card_inserted(void);
