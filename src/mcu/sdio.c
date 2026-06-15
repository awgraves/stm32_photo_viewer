#include "sdio.h"
#include "registers.h"
#include "time.h"

#define CMD0 0x0U

/*
   pg. 968 commands are fixed with 48 bits
   pg. 969 responses can be 48 (short) or 136 (long) bits.

   pg. 972 - data xfer in FIFO of 32 words (32 bits per word) = 4 x 32 = 128
  bytes operates in APB2 clock domain (PCLK2)

   pg. 975 - example of read procedure using DMA

   pg. 978 card identification process
   1. bus is activiated
   2. SDIO card host broadcasts SD_APP_OP_COND (ACMD41)
   3. response is operation condition registers from all cards
   4. incompatible cards placed in inactive state
   5. SDIO host broadcasts ALL_SEND_CID (CMD2) to all active cards
   6. active cards send their CID numbers, enter identification state
   7. SDIO card host issues SET_RELATIVE_ADDR (CMD3) to that card.
   This new address is the RCA (relative card address); shorter than CID.
   8. SDIO card host repeats for other cards (if applicable) then waits for
  timeout

  pg. 979 Block read
  CMD17 (READ_SINGLE_BLOCK) initiates a block read and after completing the
  xfer, card returns to xfer state. CMD18 (READ_MULTIPLE_BLOCK) start a xfer of
  several consecutive blocks Host can abort at any time during xfer by sending a
  STOP transmission command.

  pg. 981 wide bus selection / deselection
  4-bit bus width is selected/deselected using SET_BUS_WIDTH (ACMD6)
  default bus width after power-up or GO_IDEL_STATE (CMD0) is 1 bit.
  ACMD6 is only valid in xfer state, ie can only be changed after a card is
  selected by SELECT/DESELECT_CARD (CMD7)

  Card status register pg. 986-987
  bits 12:9 are CURRENT_STATE idle, ready, stby, tran, data etc
  bit 8: READY_FOR_DATA

  pg. 993 'Application-specific commands' (ACMD)
  when card receives APP_CMD (CMD55), it interprets subsequent cmd as an ACMD.
  Differentiates the command number from the 'general commands' (CMD).

  pg. 996 Response formats
  R1 (normal response) = 48 bits
  R2 (CID, CSD register) = 136 bits
  R3 (OCR register) = 48 bits
  R4 (Fast I/O) = 48 bits

  pg. 1002 SDIO register definitions (finally)

  !! pg. 1003, when in identification mode, SDIO_CK freq must be less than 400
  khz once relative card addresses are assigned, can increase clock speed using
  SDIO_CLKCR (sdio clock control register)

  pg. 1004, SDIO_ARG register.
  if command requires an arg, must write arg here first before writing cmd to
  cmd register. SDIO_CMD register -> command type bits control command path
  state machine CPSM.
  !!! after data write, cannot write to register again for 3 SDIOCLK clock
  periods plues 2 PCLK2 clock periods. SD cards can only send short repsonses
  (multimedia can be either short or long).

  pg. 1007 SDIO_DLEN is number of data bytes to be xfered.
  pg. 1008 SDIO_DCTL
  - bits 7:4 define data block size (1001 is for 512 bytes).
  contains DMAEN bit.
  - DTDIR sets direction card -> controller or controller -> card
  - DTEN bit enables the xfer to start
  - after a write, must wait 3 SDIOCLK (48 mhz) clock periods plues 2 PCKL2
  periods

  pg. 1009 SDIO_STA (status register)
  - read only
  - CMDSENT, CMDREND
  - bits about when FIFO is full, when data rx or tx is in progress

  pg. 1012 SDIO_MASK register configures interrupts
  pg. 1015 SDIO_FIFO registers, 32 sequential addresses of 32 bit wide
  registers. pg. 1016 SDIO register map
*/

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

  gpio_set_ospeed(conf->d0, GPIO_SPEED_HIGH);
  gpio_set_ospeed(conf->d1, GPIO_SPEED_HIGH);
  gpio_set_ospeed(conf->d2, GPIO_SPEED_HIGH);
  gpio_set_ospeed(conf->d3, GPIO_SPEED_HIGH);
  gpio_set_ospeed(conf->cmd, GPIO_SPEED_HIGH);
  gpio_set_ospeed(conf->clk, GPIO_SPEED_HIGH);
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

  sdio_reset();
}

void sdio_reset(void) {
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
  SDIO->CLKCR &= ~(SDIO_CLKCR_CLKDIV_CLEAR);
  delay_ms(1);
  switch (freq) {
  case SDIO_CK_FREQ_400KHZ:
    SDIO->CLKCR = ((SDIO->CLKCR | STARTUP_DIVISOR) & ~(SDIO_CLKCR_BYPASS));
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
  SDIO->CLKCR |= width;
  sdio_ck_enable();
  delay_ms(1);
}

static inline void sdio_cmd_flags_clear(void) {
  SDIO->ICR = SDIO_ICR_CTIMEOUT | SDIO_ICR_CCRCFAIL | SDIO_ICR_CMDREND |
              SDIO_ICR_CMDSENT;
}

static inline sdio_status_t sdio_wait_cmd_done(void) {
  uint32_t sta;
  while (1) {
    sta = SDIO->STA;

    if (sta & SDIO_STA_CMDSENT || sta & SDIO_STA_CMDREND) {
      return SDIO_OK;
    }

    if (sta & SDIO_STA_CTIMEOUT) {
      return SDIO_ERR_TIMEOUT;
    }

    if (sta & SDIO_STA_CCRCFAIL) {
      return SDIO_ERR_CRC;
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

  sdio_status_t status = sdio_wait_cmd_done();
  if (status != SDIO_OK)
    return status;

  if (resp) {
    switch (resp_type) {
    case SDIO_RESP_TYPE_SHORT:
      resp[0] = SDIO->RESP1;
      break;
    case SDIO_RESP_TYPE_LONG:
      resp[0] = SDIO->RESP1;
      resp[1] = SDIO->RESP2;
      resp[2] = SDIO->RESP3;
      resp[3] = SDIO->RESP4;
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
