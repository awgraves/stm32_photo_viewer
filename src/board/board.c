#include "board.h"
#include "drivers/ili9341.h"
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

void board_init(void) {
  sysclock_init(CPU_FREQ_40_MHZ);
  time_init();

  /* Peripherals */
  spi_config_t spi1_conf = {
      .mosi = LCD_SPI_MOSI, .sck = LCD_SPI_SCK, .baud = SPI_BAUD_DIV_2};
  spi_init(&spi1, &spi1_conf);

  /* Drivers */
  ili9341_config_t lcd_conf = {
      .spi = &spi1, .cs = LCD_CS, .dc = LCD_DC, .rst = LCD_RST};
  ili9341_init(&lcd_conf);

  rotary_encoder_config_t rot_conf = {.sw1 = ENC_CENTER,
                                      .enca = ENC_A,
                                      .encb = ENC_B,
                                      .enc_af = GPIO_AF_TIM2,
                                      .timer = &timer2};
  rotary_encoder_init(&rot_conf);

  sd_card_config_t sd_conf = {
      .det = SD_DET,
  };
  sd_card_init(&sd_conf);
}
