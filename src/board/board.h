#pragma once
#include "../mcu/gpio.h"
#include "../mcu/spi.h"

#define LED_PIN PA5

/*
For LCD
| Function  | Pin |
| --------- | --- |
| SPI1_SCK  | PA5 |
| SPI1_MOSI | PA7 |
| LCD_CS    | PA9 |
| LCD_DC    | PA8 |
| LCD_RST   | PA10 |
*/
#define LCD_SPI spi1
#define LCD_SPI_SCK PA5
#define LCD_SPI_MOSI PA7
#define LCD_CS PA9
#define LCD_DC PA8
#define LCD_RST PA10
