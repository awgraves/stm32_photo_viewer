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
  CARD_ERR_FAILED_VOLTAGE,
  CARD_ERR_FAILED_HANDSHAKE,
  CARD_ERR_NOT_HC_CARD,
  CARD_ERR_FAILED_CID,
  CARD_ERR_FAILED_RCA,
  CARD_ERR_FAILED_CSD,
  CARD_ERR_FAILED_SELECTION,
  CARD_ERR_FAILED_BUS_WIDTH_CHANGE,
  CARD_ERR_FAILED_SPEED_INCREASE_CONFIRMATION,
} card_result_t;

typedef struct {
  char name[6];
  uint16_t rca;
  uint64_t sector_count;
  uint64_t capacity_bytes;
} sd_card_info_t;

void sd_card_gpio_init(sd_card_gpio_config_t *config);

bool sd_card_inserted(void);
card_result_t sd_card_initialize(void);

const sd_card_info_t *sd_card_get_info(void);
