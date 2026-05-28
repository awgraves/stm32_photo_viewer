#pragma once
#include "gpio.h"
#include <stdint.h>

typedef struct spi spi_t;
extern spi_t spi1;

// RM0390 pg. 866
typedef enum {
  SPI_BAUD_DIV_2,
  // others unused atm
} spi_baud_t;

typedef struct spi_config {
  gpio_pin_t mosi, sck;
  spi_baud_t baud;
} spi_config_t;

void spi_init(spi_t *spi, spi_config_t *c);
void spi_tx(spi_t *spi, const uint8_t data[], uint16_t len);
