#include "spi.h"
#include "gpio.h"
#include "registers.h"

struct spi {
  SPI_t *regs;
  gpio_af_t af;
  gpio_pin_t mosi, sck;
};

spi_t spi1 = {.regs = SPI1, .af = GPIO_AF_SPI1, 0, 0};

static void spi_clock_enable(SPI_t *regs);

void spi_init(spi_t *spi, gpio_pin_t mosi, gpio_pin_t sck) {
  gpio_set_mode(mosi, GPIO_MODE_AF);
  gpio_set_mode(sck, GPIO_MODE_AF);

  gpio_set_AF(mosi, spi->af);
  gpio_set_AF(sck, spi->af);

  spi_clock_enable(spi->regs);

  spi->regs->CR1 |= (SPI_MASTER | SPI_SSM | SPI_SSI | SPI_ENABLE);
}

void spi_tx(spi_t *spi, const uint8_t data[], uint32_t size) {
  while (spi->regs->SR & (SPI_BSY))
    ;

  uint32_t i = 0;
  while (i < size) {
    while (!(spi->regs->SR & (SPI_TXE)))
      ;
    spi->regs->DR = data[i++];
  }

  while (!(spi->regs->SR & (SPI_TXE)))
    ;

  while (spi->regs->SR & (SPI_BSY))
    ;

  // clear OVR flag
  i = spi->regs->DR;
  i = spi->regs->SR;
};

static void spi_clock_enable(SPI_t *regs) {
  if (regs == SPI1)
    RCC->APB2ENR |= APB2ENR_SPI1;
}
