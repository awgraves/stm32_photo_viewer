#pragma once
#include "mcu/gpio.h"
#include "mcu/sdio.h"
#include <stdbool.h>

typedef struct {
  gpio_pin_t det;
  sdio_config_t sdio;
} sd_card_gpio_config_t;

typedef enum {
  CARD_OK,
  CARD_ERR_NOT_INSERTED,
  CARD_ERR_FAILED_RESET,
  CARD_ERR_FAILED_VOLTAGE
} card_result_t;

void sd_card_gpio_init(sd_card_gpio_config_t *config);

bool sd_card_inserted(void);
card_result_t sd_card_initialize(void);
