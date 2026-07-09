#include "spi.h"
#include "dma.h"
#include "gpio.h"
#include "registers.h"

#define DMA_THRESHOLD 2       // keep polling for small tx, ie commands
#define SPI_DMA_MAX_LEN 65535 // per RM0390 pg. 228

typedef struct {
  DMA_t *regs;
  uint32_t channel_select;
  uint32_t tx_stream_tcif;
  uint32_t tx_clear_flags;
  uint8_t tx_stream_idx;
} dma_t;

struct spi {
  SPI_t *regs;
  gpio_af_t af;

  dma_t dma;
};

spi_t spi1 = {.regs = SPI1,
              .af = GPIO_AF_SPI1,
              .dma = {/*
                        RM0390 pg. 204
                        spi1 tx/rx are on DMA2 channel 3
                        tx can be stream 3 or 5 (chose 3)
                      */
                      .regs = DMA2,
                      .channel_select = DMA_SxCR_CHSEL_3,
                      .tx_stream_idx = 3,
                      .tx_stream_tcif = DMA_LISR_STREAM_3_TCIF,
                      .tx_clear_flags = DMA_LIFCR_STREAM_3_ALL_FLAGS}};

/*
  Initialization
*/

static inline void spi_clock_enable(spi_t *spi) {
  if (spi->regs == SPI1)
    RCC->APB2ENR |= RCC_APB2ENR_SPI1;
}

static void spi_dma_init(spi_t *spi);

void spi_init(spi_t *spi, spi_config_t *c) {
  gpio_set_mode(c->mosi, GPIO_MODE_AF);
  gpio_set_mode(c->sck, GPIO_MODE_AF);

  gpio_set_AF(c->mosi, spi->af);
  gpio_set_AF(c->sck, spi->af);

  // the GPIO speed vals DO matter depending on the configured spi bus speed.
  gpio_set_ospeed(c->mosi, c->gpio_speed);
  gpio_set_ospeed(c->sck, c->gpio_speed);

  spi_clock_enable(spi);

  spi_dma_init(spi);

  // RM 0393 pg. 865
  // keeping defaults for now of:
  // Clock phase 0, Clock polarity 0
  uint8_t baud = ((c->baud & 0x7) << 3);
  spi->regs->CR1 = (SPI_CR1_MASTER | SPI_CR1_SSM | SPI_CR1_SSI | baud);
  spi->regs->CR1 |= SPI_CR1_ENABLE;
}

static inline void dma_clock_enable(dma_t *dma) {
  if (dma->regs == DMA2)
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA2;
}

static inline DMA_stream_t *dma_get_tx_stream(dma_t *dma) {
  return &dma->regs->STREAM[dma->tx_stream_idx];
}

void spi_dma_init(spi_t *spi) {
  dma_clock_enable(&spi->dma);

  /*
  following pg. 219 for tx config procedure
  */
  DMA_stream_t *stream = dma_get_tx_stream(&spi->dma);
  dma_stream_disable(stream);

  // set the periph address to SPI data register
  stream->PAR = (uint32_t)&spi->regs->DR;

  // keeping other defaults:
  // single xfers, no double buffer, low priority, 8-bit data sizes
  stream->CR = (spi->dma.channel_select | DMA_SxCR_INCR_MEM |
                DMA_SxCR_DIR_MEM_TO_PERIPH);

  // stream will be enabled again prior to tx start
}

/*
  TX Functionality
*/

static void spi_tx_polling(spi_t *spi, const uint8_t data[], uint16_t len);
static void spi_tx_dma(spi_t *spi, const uint8_t data[], uint32_t len);

void spi_tx(spi_t *spi, const uint8_t data[], uint32_t len) {
  // polling is faster with small data
  // otherwise DMA is faster
  len > DMA_THRESHOLD ? spi_tx_dma(spi, data, len)
                      : spi_tx_polling(spi, data, len);
}

static void spi_tx_polling(spi_t *spi, const uint8_t data[], uint16_t len) {
  while (spi->regs->SR & (SPI_SR_BSY))
    ;

  uint32_t i = 0;
  while (i < len) {
    while (!(spi->regs->SR & (SPI_SR_TXE)))
      ;
    spi->regs->DR = data[i++];
  }

  while (spi->regs->SR & (SPI_SR_BSY))
    ;

  // clear OVR flag
  // RM 0390 pg. 843
  // doesn't matter when only in tx mode, but later rx may be used
  i = spi->regs->DR;
  i = spi->regs->SR;
}

// ISR: Interrupt State Register
static inline volatile uint32_t *get_dma_isr(dma_t *dma, uint8_t stream_idx) {
  return stream_idx < 4 ? &dma->regs->LISR : &dma->regs->HISR;
}

// IFCR: Interrupt Flag Clear Register
static inline volatile uint32_t *get_dma_ifcr(dma_t *dma, uint8_t stream_idx) {
  return stream_idx < 4 ? &dma->regs->LIFCR : &dma->regs->HIFCR;
}

static inline void spi_tx_dma_enable(spi_t *spi) {
  spi->regs->CR2 |= SPI_CR2_TXDMAEN;
}

static inline void spi_tx_dma_disable(spi_t *spi) {
  spi->regs->CR2 &= ~(SPI_CR2_TXDMAEN);
}

static void spi_tx_dma_chunk(spi_t *spi, const uint8_t data[], uint16_t len) {
  while (spi->regs->SR & (SPI_SR_BSY))
    ;

  dma_t *dma = &spi->dma;
  DMA_stream_t *stream = dma_get_tx_stream(dma);

  // ensure stream is disabled & flag state is clear
  // stream must be disabled before HW allows config changes
  dma_stream_disable(stream);
  *get_dma_ifcr(dma, dma->tx_stream_idx) = dma->tx_clear_flags;

  // set mem source to data
  stream->M0AR = (uint32_t)data;
  // set num data items to xfer, limited to uint16 size
  stream->NDTR = len;

  spi_tx_dma_enable(spi);
  dma_stream_enable(stream);

  // wait for xfer to complete
  volatile uint32_t *isr_reg = get_dma_isr(dma, dma->tx_stream_idx);
  while (!(*isr_reg & dma->tx_stream_tcif))
    ;
  while (spi->regs->SR & SPI_SR_BSY)
    ;

  dma_stream_disable(stream);
  spi_tx_dma_disable(spi);
}

static void spi_tx_dma(spi_t *spi, const uint8_t data[], uint32_t len) {
  uint32_t sent = 0;
  while (sent < len) {
    uint32_t remaining = len - sent;
    uint16_t chunk = remaining < SPI_DMA_MAX_LEN ? remaining : SPI_DMA_MAX_LEN;
    spi_tx_dma_chunk(spi, &data[sent], chunk);
    sent += chunk;
  }
}
