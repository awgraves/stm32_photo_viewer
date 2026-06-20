#pragma once
#include "mcu/gpio.h"
#include "mcu/sdio.h"
#include <stdbool.h>

// block = sector = 512 bytes
#define BLOCK_SIZE_IN_BYTES 512ULL // sd card spec page. 234

typedef struct {
  gpio_pin_t det;
  sdio_config_t sdio;
} sd_card_gpio_config_t;

typedef enum {
  CARD_OK,
  // init errors
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
  // read errors
  CARD_ERR_SECTOR_OUT_OF_BOUNDS,
  CARD_ERR_FAILED_READ,
} card_result_t;
typedef struct {
  char name[6];
  uint16_t rca;
  uint32_t sector_count;
  uint64_t capacity_bytes;
} sd_card_info_t;

void sd_card_gpio_init(sd_card_gpio_config_t *config);

bool sd_card_inserted(void);
card_result_t sd_card_initialize(void);

card_result_t sd_card_read_sector(uint32_t sector_num,
                                  uint8_t buff[BLOCK_SIZE_IN_BYTES]);

const sd_card_info_t *sd_card_get_info(void);
