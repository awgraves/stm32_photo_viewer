#pragma once
#include "gpio.h"

typedef struct {
  gpio_pin_t d0;
  gpio_pin_t d1;
  gpio_pin_t d2;
  gpio_pin_t d3;
  gpio_pin_t cmd;
  gpio_pin_t clk;
} sdio_config_t;

typedef enum {
  SDIO_CK_FREQ_400KHZ,
  SDIO_CK_FREQ_24MHZ,
  SDIO_CK_FREQ_48MHZ,
} sdio_ck_freq_t;

// RM 0390 pg. 1003
typedef enum { SDIO_BUS_WIDTH_1, SDIO_BUS_WIDTH_4 } sdio_bus_width_t;

void sdio_init(sdio_config_t *conf);
void sdio_reset_bus_speed_and_width(void);
void sdio_ck_freq_set(sdio_ck_freq_t freq);
void sdio_bus_width_set(sdio_bus_width_t width);

// RM 0390 pgs. 996 - 1000
typedef enum {
  SDIO_RESP_TYPE_NONE,
  SDIO_RESP_TYPE_SHORT,        // 48 bits: R1, R4
  SDIO_RESP_TYPE_SHORT_NO_CRC, // special case for R3 (OCR) response
  SDIO_RESP_TYPE_LONG,         // 136 bits: R2 (CID/CSD)
} sdio_resp_type_t;

typedef enum {
  SDIO_OK,
  SDIO_ERR_TIMEOUT,
  SDIO_ERR_CRC,
  SDIO_ERR_CMD
} sdio_status_t;

sdio_status_t sdio_send_cmd(uint8_t cmd, uint32_t arg,
                            sdio_resp_type_t resp_type, uint32_t *resp);
