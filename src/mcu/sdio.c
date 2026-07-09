#include "sdio.h"
#include "dma.h"
#include "registers.h"
#include "time.h"

static inline void sdio_dma_init(void);

void sdio_init(sdio_config_t *conf) {
  gpio_set_mode(conf->d0, GPIO_MODE_AF);
  gpio_set_mode(conf->d1, GPIO_MODE_AF);
  gpio_set_mode(conf->d2, GPIO_MODE_AF);
  gpio_set_mode(conf->d3, GPIO_MODE_AF);
  gpio_set_mode(conf->cmd, GPIO_MODE_AF);
  gpio_set_mode(conf->clk, GPIO_MODE_AF);

  gpio_set_AF(conf->d0, GPIO_AF_SDIO);
  gpio_set_AF(conf->d1, GPIO_AF_SDIO);
  gpio_set_AF(conf->d2, GPIO_AF_SDIO);
  gpio_set_AF(conf->d3, GPIO_AF_SDIO);
  gpio_set_AF(conf->cmd, GPIO_AF_SDIO);
  gpio_set_AF(conf->clk, GPIO_AF_SDIO);

  // There was signal integrity issues when speed set higher
  // unclear if can/should increase speed once on a PCB?
  gpio_set_ospeed(conf->d0, GPIO_SPEED_MEDIUM);
  gpio_set_ospeed(conf->d1, GPIO_SPEED_MEDIUM);
  gpio_set_ospeed(conf->d2, GPIO_SPEED_MEDIUM);
  gpio_set_ospeed(conf->d3, GPIO_SPEED_MEDIUM);
  gpio_set_ospeed(conf->cmd, GPIO_SPEED_MEDIUM);
  gpio_set_ospeed(conf->clk, GPIO_SPEED_MEDIUM);
  /*
    My Adafruit sd card module already has external pullup resistors,
    but adding some internal ones here as fallbacks in case future board
    revisions lack them.
  */
  gpio_set_pupd(conf->d0, GPIO_PUPD_PULL_UP);
  gpio_set_pupd(conf->d1, GPIO_PUPD_PULL_UP);
  gpio_set_pupd(conf->d2, GPIO_PUPD_PULL_UP);
  gpio_set_pupd(conf->d3, GPIO_PUPD_PULL_UP);
  gpio_set_pupd(conf->cmd, GPIO_PUPD_PULL_UP);

  /*
    See RM0390 pg. 964 for SDIO clock diagram
    SDIOCLK is PLL48mhz + prescaler
    PCLK2 is the APB2 bus clock
    SDIO_CK is the clock signal output to the sd card over clk pin
  */
  // enable clock to SDIO interface on (APB2 bus)
  RCC->APB2ENR |= RCC_APB2ENR_SDIO;
  // power up the SDIO state machine
  SDIO->POWER = SDIO_POWER_ON;

  sdio_bus_speed_and_width_reset();

  // placing this here for now. RM0390 pg. 1006
  SDIO->DTIMER = 0xFFFFFFFF; // max timeout
  SDIO->DLEN = 512;          // will always remain constant, 512 is block size

  sdio_dma_init();
}

static inline void sdio_dma_init(void) { RCC->AHB1ENR |= RCC_AHB1ENR_DMA2; }

static inline DMA_stream_t *get_dma_stream(void) { return &DMA2->STREAM[6]; }

#define CMD17 0x11U // read 1 block
sdio_status_t sdio_read_block(uint32_t sector_num, uint8_t buff[512]) {
  /*
    RM0390 pg. 975, READ with DMA
    1. set SDIO data length register
    2. program the DMA channel
    3. program SDIO DCTL, DTEN with 1, DTDIR with 1, DTMODE 0, DMAEN 1,
    DBLOCKSIZE with 0x9 (512 bytes)
    4. SDIO arg register with address location of data on card
    5. SDIO command register CMD17, wait for resp ('1'), CPSMEN.
    6. wait for CMDREND[6] on STA
    7. wait for DBCKEND[10] on STA
    8. wait until FIFO empty, RXOVERR[5]

    RM0390 pg. 976 DMA config
    1. enable DMA2, clear any pending interrupts
    2. choice between DMA2_Stream3 (or 6) channel 4 for memory location and
    DMA2_stream3 (or 6) for channel 4 destination
    3. dma2_stream3 channel 4 control reg
    4. dma2_stream3 channel 4 select peripheral as flow controller
    5. configure incremental burst transfer to 4 beats (at least from
    peripherlal side)
    6. enable dma2_stream3 channel 4
    Note 1: must use DMA in periph flow controller mode.
    Note 2: sdio generates only DMA burst requests. DMA must be configured in
    incremental burst mode on periph side

    pg. 215 FIFO burst config combos
  */

  SDIO->ICR = 0xFFFFFFFF;          // clear all SDIO status flags
  SDIO->DCTRL = (SDIO_DCTRL_DTEN | //
                 SDIO_DCTRL_DTDIR_FROM_CARD_TO_CONTROLLER | //
                 SDIO_DCTRL_DMAEN |                         //
                 SDIO_DCTRL_DBLOCKSIZE_512);

  // clear any existing flags
  DMA2->HIFCR = DMA_HIFCR_STREAM_6_ALL_FLAGS;

  DMA_stream_t *stream = get_dma_stream();
  dma_stream_disable(stream);

  stream->PAR = (uint32_t)&SDIO->FIFO;
  stream->M0AR = (uint32_t)buff;
  stream->NDTR = 128; // 512 bytes block size / 4 bytes per word = 128

  stream->CR = (DMA_SxCR_CHSEL_4 |           //
                DMA_SxCR_DIR_PERIPH_TO_MEM | //
                DMA_SxCR_PBURST_4_BEATS |    //
                DMA_SxCR_MBURST_4_BEATS |    //
                DMA_SxCR_PSIZE_WORD |        //
                DMA_SxCR_MSIZE_WORD |        //
                DMA_SxCR_INCR_MEM |          //
                DMA_SxCR_PERIPH_CONTROLS_FLOW);

  stream->FCR = (DMA_SxFCR_DMDIS | DMA_SxFCR_FIFO_THRESHOLD_FULL);

  dma_stream_enable(stream);

  sdio_status_t status;
  uint32_t resp;
  status = sdio_send_cmd(CMD17, sector_num, SDIO_RESP_TYPE_SHORT, &resp);
  if (status != SDIO_OK) {
    return status;
  }

  volatile uint32_t sta;
  volatile uint32_t dma2_hisr;
  while (1) {
    sta = SDIO->STA;
    dma2_hisr = DMA2->HISR;

    // SDIO errors
    if (sta & SDIO_STA_DCRCFAIL) {
      status = SDIO_ERR_DATA_INTEGRITY;
      break;
    } else if (sta & SDIO_STA_RXOVERR) {
      status = SDIO_ERR_FIFO_OVERRUN;
      break;
    } else if (sta & SDIO_STA_DTIMEOUT) {
      status = SDIO_ERR_TIMEOUT;
      break;
    }

    // success
    if (dma2_hisr & DMA_HISR_STREAM_6_TCIF) {
      status = SDIO_OK;
      break;
    }
  }

  return status;
}

void sdio_bus_speed_and_width_reset(void) {
  // freq must be slow during identification phase
  sdio_ck_freq_set(SDIO_CK_FREQ_400KHZ);
  sdio_bus_width_set(SDIO_BUS_WIDTH_1);
}

static inline void sdio_ck_disable(void);
static inline void sdio_ck_enable(void);

// RM0390 pg. 1003
// SDIO_CK needs to be 400 khz or less during card identification phase
// formula: freq = SDIOCLK (ie PLL48mhz) / [CLKDIV + 2]
#define SDIOCLK_HZ 48000000U
#define SDIO_INIT_HZ 400000U
#define STARTUP_DIVISOR ((SDIOCLK_HZ / SDIO_INIT_HZ) - 2)

void sdio_ck_freq_set(sdio_ck_freq_t freq) {
  sdio_ck_disable();
  SDIO->CLKCR &= ~(SDIO_CLKCR_CLKDIV_CLEAR | SDIO_CLKCR_BYPASS);
  delay_ms(1);
  switch (freq) {
  case SDIO_CK_FREQ_400KHZ:
    SDIO->CLKCR = (SDIO->CLKCR | STARTUP_DIVISOR);
    delay_ms(1);
    break;
  case SDIO_CK_FREQ_24MHZ:
    // default CLKDIV is 0, so PLL48mhz / (0 + 2)
    SDIO->CLKCR &= ~(0xFF);
    delay_ms(1);
    break;
  case SDIO_CK_FREQ_48MHZ:
    SDIO->CLKCR |= SDIO_CLKCR_BYPASS;
    break;
  }
  sdio_ck_enable();
  delay_ms(1);
}

void sdio_bus_width_set(sdio_bus_width_t width) {
  sdio_ck_disable();
  SDIO->CLKCR &= ~(SDIO_CLKCR_BUS_WIDTH_CLEAR_BITS);
  delay_ms(1);
  SDIO->CLKCR |= SDIO_CLKCR_BUS_WIDTH(width);
  sdio_ck_enable();
  delay_ms(1);
}

static inline void sdio_cmd_flags_clear(void) {
  SDIO->ICR = SDIO_ICR_CTIMEOUTC | SDIO_ICR_CCRCFAILC | SDIO_ICR_CMDRENDC |
              SDIO_ICR_CMDSENTC;
}

static inline sdio_status_t sdio_wait_cmd_done(sdio_resp_type_t type) {
  uint32_t sta;
  while (1) {
    sta = SDIO->STA;

    if (type == SDIO_RESP_TYPE_NONE) {
      if (sta & SDIO_STA_CMDSENT) {
        return SDIO_OK;
      }
    }

    if (sta & SDIO_STA_CMDREND) {
      return SDIO_OK;
    }

    if (sta & SDIO_STA_CTIMEOUT) {
      return SDIO_ERR_TIMEOUT;
    }

    if (sta & SDIO_STA_CCRCFAIL) {
      // STM32 SDIO is 'dumb', requires software exception case
      // for SD R3 type responses which always fail CRC checks.
      // https://community.st.com/stm32-mcus-products-25/sdio-command-crc-error-on-acmd41-114671
      // also noted at bottom of RM0390, pg. 968
      return type == SDIO_RESP_TYPE_SHORT_NO_CRC ? SDIO_OK : SDIO_ERR_CRC;
    }
  }
}

sdio_status_t sdio_send_cmd(uint8_t cmd, uint32_t arg,
                            sdio_resp_type_t resp_type, uint32_t *resp) {
  while (SDIO->STA & SDIO_STA_CMDACT)
    ;
  sdio_cmd_flags_clear();

  SDIO->ARG = arg;

  uint32_t tmp_cmd = (cmd & 0x3F);
  switch (resp_type) {
  case SDIO_RESP_TYPE_NONE:
    tmp_cmd |= SDIO_CMD_WAIT_RESP_NONE;
    break;
  case SDIO_RESP_TYPE_SHORT:
  case SDIO_RESP_TYPE_SHORT_NO_CRC:
    tmp_cmd |= SDIO_CMD_WAIT_RESP_SHORT;
    break;
  case SDIO_RESP_TYPE_LONG:
    tmp_cmd |= SDIO_CMD_WAIT_RESP_LONG;
    break;
  default:
    // unreachable
    break;
  }

  SDIO->CMD = tmp_cmd | SDIO_CMD_CPSMEN;

  sdio_status_t status = sdio_wait_cmd_done(resp_type);
  if (status != SDIO_OK)
    return status;

  if (resp) {
    switch (resp_type) {
    case SDIO_RESP_TYPE_SHORT:
    case SDIO_RESP_TYPE_SHORT_NO_CRC:
      resp[0] = SDIO->RESP1;
      break;
    case SDIO_RESP_TYPE_LONG:
      // RM 0390 pg. 1006, RESP1 contains MSB, 127:96, etc
      // but in memory each 32 bits is stored in little endian
      // so must swap to achieve same byte order as what came over the wire
      // and then cast to a uint8_t stream
      resp[0] = __builtin_bswap32(SDIO->RESP1);
      resp[1] = __builtin_bswap32(SDIO->RESP2);
      resp[2] = __builtin_bswap32(SDIO->RESP3);
      resp[3] = __builtin_bswap32(SDIO->RESP4);
      break;
    default:
      break;
    }
  }

  return SDIO_OK;
}

static inline void sdio_ck_disable(void) {
  SDIO->CLKCR &= ~(SDIO_CLKCR_CLKEN);
  delay_ms(1);
}
static inline void sdio_ck_enable(void) {
  SDIO->CLKCR |= SDIO_CLKCR_CLKEN;
  delay_ms(1);
}
