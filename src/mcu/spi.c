#include "spi.h"
#include "gpio.h"
#include "registers.h"

struct spi {
  SPI_t *regs;
  gpio_af_t af;
};

spi_t spi1 = {.regs = SPI1, .af = GPIO_AF_SPI1};

static void spi_clock_enable(SPI_t *regs);

void spi_init(spi_t *spi, spi_config_t *c) {
  gpio_set_mode(c->mosi, GPIO_MODE_AF);
  gpio_set_mode(c->sck, GPIO_MODE_AF);

  gpio_set_AF(c->mosi, spi->af);
  gpio_set_AF(c->sck, spi->af);

  spi_clock_enable(spi->regs);

  uint8_t baud = ((c->baud & 0x7) << 3);
  // RM 0393 pg. 865
  // keeping defaults for now of:
  // Clock phase 0, Clock polarity 0
  spi->regs->CR1 = (SPI_MASTER | SPI_SSM | SPI_SSI | baud);
  spi->regs->CR1 |= SPI_ENABLE;
}

void spi_tx(spi_t *spi, const uint8_t data[], uint32_t len) {
  while (spi->regs->SR & (SPI_BSY))
    ;

  uint32_t i = 0;
  while (i < len) {
    while (!(spi->regs->SR & (SPI_TXE)))
      ;
    spi->regs->DR = data[i++];
  }

  while (spi->regs->SR & (SPI_BSY))
    ;

  if (spi->regs->SR & SPI_RXNE) {
    // clear OVR flag
    // RM 0390 pg. 843
    i = spi->regs->DR;
    i = spi->regs->SR;
  }
};

static void spi_clock_enable(SPI_t *regs) {
  if (regs == SPI1)
    RCC->APB2ENR |= APB2ENR_SPI1;
}
