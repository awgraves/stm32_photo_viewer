#pragma once
#include "mcu/gpio.h"
#include "mcu/sdio.h"
#include <stdbool.h>

typedef struct {
  gpio_pin_t det;
  sdio_config_t sdio;
} sd_card_config_t;

typedef enum {
  CARD_ERR_NONE,
  CARD_ERR_NOT_INSERTED,
  CARD_ERR_FAILED_RESET,
  CARD_ERR_FAILED_VOLTAGE
} card_err_t;

typedef struct {
  card_err_t err;
} sd_card_state_t;

void sd_card_init(sd_card_config_t *config);

bool sd_card_inserted(void);
bool sd_card_probe(void);
const sd_card_state_t *sd_card_get_state(void);
