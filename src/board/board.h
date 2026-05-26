#pragma once
#include "mcu/gpio.h"
#include "mcu/spi.h"

#define LCD_SPI ((spi_t)spi1)
#define LCD_SPI_SCK (PA5)
#define LCD_SPI_MOSI (PA7)
#define LCD_CS (PA9)
#define LCD_DC (PA8)
#define LCD_RST (PA10)
