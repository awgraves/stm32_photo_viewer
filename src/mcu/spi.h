#pragma once
#include "gpio.h"
#include <stdint.h>

typedef struct spi spi_t;
extern spi_t spi1;

void spi_init(spi_t *spi, gpio_pin_t mosi, gpio_pin_t sck);
void spi_tx(spi_t *spi, const uint8_t data[], uint32_t size);
