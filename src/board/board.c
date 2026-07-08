#include "board.h"
#include "drivers/display.h"
#include "drivers/rotary_encoder.h"
#include "drivers/sd_card.h"
#include "mcu/spi.h"
#include "mcu/sysclock.h"
#include "mcu/time.h"

#define LCD_SPI_SCK PA5
#define LCD_SPI_MOSI PA7
#define LCD_CS PA9
#define LCD_DC PA8
#define LCD_RST PA10
#define LCD_BL PA11

#define ENC_A PA0 // TIM2_CH1 AF datasheet pg 46
#define ENC_B PA1 // TIM2_CH2 AF
#define ENC_CENTER PA4

// datasheet AF table pg. 59-60
#define SDIO_D0 PC8
#define SDIO_D1 PC9
#define SDIO_D2 PC10
#define SDIO_D3 PC11
#define SDIO_CLK PC12
#define SDIO_CMD PD2
#define SD_DET PC6

/*
Note that SWDIO is on PA13
and SWDCLK is on PA14

also note nucleo board manual pg. 36 for board pin layout
*/

void board_init(void) {
  sysclock_init(CPU_FREQ_60_MHZ);
  time_init();

  /* Peripherals */
  spi_config_t spi1_conf = {
      .mosi = LCD_SPI_MOSI, .sck = LCD_SPI_SCK, .baud = SPI_BAUD_DIV_2};
  spi_init(&spi1, &spi1_conf);

  /* Drivers */
  display_config_t display_conf = {
      .spi = &spi1, .cs = LCD_CS, .dc = LCD_DC, .rst = LCD_RST, .bl = LCD_BL};
  display_init(&display_conf);

  rotary_encoder_config_t rot_conf = {.sw1 = ENC_CENTER,
                                      .enca = ENC_A,
                                      .encb = ENC_B,
                                      .enc_af = GPIO_AF_TIM2,
                                      .timer = &timer2};
  rotary_encoder_init(&rot_conf);

  sd_card_gpio_config_t sd_conf = {.det = SD_DET,
                                   .sdio = {
                                       .d0 = SDIO_D0,
                                       .d1 = SDIO_D1,
                                       .d2 = SDIO_D2,
                                       .d3 = SDIO_D3,
                                       .clk = SDIO_CLK,
                                       .cmd = SDIO_CMD,
                                   }};
  sd_card_gpio_init(&sd_conf);
}
