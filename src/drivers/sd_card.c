#include "sd_card.h"
#include "mcu/registers.h"
#include "mcu/time.h"
#include <stdbool.h>

static sd_card_gpio_config_t conf;
static sd_card_info_t card;

void sd_card_gpio_init(sd_card_gpio_config_t *config) {
  conf = *config;
  gpio_set_mode(conf.det, GPIO_MODE_INPUT);

  sdio_init(&conf.sdio);
}

bool sd_card_inserted(void) { return gpio_digital_read(conf.det); }

// sd spec pg. 224 outlines CID register
#define CID_PNM_MSB 103
#define CID_PNM_LSB 64

// sd spec pg. 226 outlines CSD register version 2.0 (for SDHC and SDXC)
#define CSD_C_SIZE_MSB 69
#define CSD_C_SIZE_LSB 48

/*
  Commands and sequencing comes from "SD spec physical layer simplified"
  https://www.sdcard.org/downloads/pls/pdf/?p=Part1_Physical_Layer_Simplified_Specification_Ver9.10.jpg&f=Part1PhysicalLayerSimplifiedSpecificationVer9.10Fin_20231201.pdf&e=EN_SS9_1
  beginning pg. 41
*/
#define CMD0 0x0U              // go idle
#define CMD8 0x8U              // check voltage compatibility (expect echo)
#define ARG_VOLTAGE_VAL 0x1AAU // 2.7v to 3.6v
#define CMD55 0x37U            // next cmd sent should be interpreted as ACMD
#define ACMD41 0x29U           // ops setting negotiation between host and card
#define ACMD41_VOLTAGE_WINDOW 0x00FF8000U // 2.7v - 3.6v (standard range)
#define ACMD41_HCS (1U << 30)             // can handle high capacity cards
#define ARG_ACMD41 (ACMD41_VOLTAGE_WINDOW | ACMD41_HCS)
#define CARD_INITIALIZED (1U << 31)  // from successful ACMD41 response
#define HIGH_CAPACITY_BIT (1U << 30) // from successfuly ACMD41 response
#define CMD2 0x2U                    // request CID (card identification data)
#define CMD3 0x3U                    // request RCA (relative card address)
#define CMD9 0x9U                    // request CSD (card-specific data)
#define CMD7 0x7U                    // select card, place in transfer mode
#define CMD13 0xDU                   // get current card state
#define CARD_TRANSFER_MODE 0x4U // 4 bit status num 12:9 in CMD13 resp, pg. 122
#define ACMD6 0x6U              // change bus width to 4 bit spec pg. 52
#define ACMD6_ARG_4_BITS 0x2U   // 4 bit width

#define CMD17 0x11U // read 1 block

static inline bool is_high_capacity_card(uint32_t raw_resp);
static inline void save_product_name(const uint32_t raw_resp[4]);
static inline void save_rca(uint32_t raw_resp);
static inline void save_capacity_info(const uint32_t raw_resp[4]);

/*
  Note, for simplicity of this driver, choosing to only support modern 'high
  capacity' sd cards, which are 4 GB to 32 GB in size and SDXC (extended
  capability) which are > 64GB.
  Excluding the legacy 'standrad capacity' ones of < 2GB.
*/
card_result_t sd_card_initialize(void) {
  if (!sd_card_inserted()) {
    return CARD_ERR_NOT_INSERTED;
  }
  sdio_bus_speed_and_width_reset();

  // software reset
  sdio_status_t status;
  status = sdio_send_cmd(CMD0, 0, SDIO_RESP_TYPE_NONE, 0);
  if (status != SDIO_OK) {
    return CARD_ERR_FAILED_RESET;
  }

  // check voltage
  uint32_t short_resp;
  status =
      sdio_send_cmd(CMD8, ARG_VOLTAGE_VAL, SDIO_RESP_TYPE_SHORT, &short_resp);
  if (status != SDIO_OK || ((short_resp & 0xFFF) != ARG_VOLTAGE_VAL)) {
    return CARD_ERR_FAILED_VOLTAGE;
  }

  uint32_t start = millis();
  do {
    // SD spec pg. 43, card should respond to handshake within 1 second.
    if (millis() - start >= 1000) {
      return CARD_ERR_FAILED_HANDSHAKE;
    }

    status = sdio_send_cmd(CMD55, 0, SDIO_RESP_TYPE_SHORT, &short_resp);
    if (status != SDIO_OK) {
      return CARD_ERR_FAILED_HANDSHAKE;
    }

    status = sdio_send_cmd(ACMD41, ARG_ACMD41, SDIO_RESP_TYPE_SHORT_NO_CRC,
                           &short_resp);
    if (status != SDIO_OK) {
      return CARD_ERR_FAILED_HANDSHAKE;
    }
  } while (!(short_resp & CARD_INITIALIZED));

  if (!is_high_capacity_card(short_resp)) {
    return CARD_ERR_NOT_HC_CARD; // modern cards, 2gb or more
  }

  uint32_t long_resp[4];
  status = sdio_send_cmd(CMD2, 0, SDIO_RESP_TYPE_LONG, long_resp);
  if (status != SDIO_OK) {
    return CARD_ERR_FAILED_CID;
  }

  save_product_name(long_resp);

  status = sdio_send_cmd(CMD3, 0, SDIO_RESP_TYPE_SHORT, &short_resp);
  if (status != SDIO_OK) {
    return CARD_ERR_FAILED_RCA;
  }
  save_rca(short_resp);

  status = sdio_send_cmd(CMD9, card.rca << 16, SDIO_RESP_TYPE_LONG, long_resp);
  if (status != SDIO_OK) {
    return CARD_ERR_FAILED_CSD;
  }
  save_capacity_info(long_resp);

  /*
    Place card in transfer mode
  */
  status =
      sdio_send_cmd(CMD7, card.rca << 16, SDIO_RESP_TYPE_SHORT, &short_resp);
  if (status != SDIO_OK) {
    return CARD_ERR_FAILED_SELECTION;
  }
  status =
      sdio_send_cmd(CMD13, card.rca << 16, SDIO_RESP_TYPE_SHORT, &short_resp);
  if (status != SDIO_OK || (((short_resp >> 9) & 0xF) != CARD_TRANSFER_MODE)) {
    return CARD_ERR_FAILED_SELECTION;
  }

  /*
    Set bus width to 4

    note: seeing signal integrity issues when prototyping with long jumper
    wires. will keep bus width to 1 until working with PCB, then can try again.
  */
  // sdio_bus_width_set(SDIO_BUS_WIDTH_4);
  // status =
  //     sdio_send_cmd(CMD55, card.rca << 16, SDIO_RESP_TYPE_SHORT,
  //     &short_resp);
  // if (status != SDIO_OK) {
  //   return CARD_ERR_FAILED_BUS_WIDTH_CHANGE;
  // }

  status =
      sdio_send_cmd(ACMD6, ACMD6_ARG_4_BITS, SDIO_RESP_TYPE_SHORT, &short_resp);
  if (status != SDIO_OK) {
    return CARD_ERR_FAILED_BUS_WIDTH_CHANGE;
  }

  /*
    Increase bus speed.

    Note: sticking with 24mhz instead of full 48mhz to keep driver simpler.
    Not all cards can move at 'high speed' and complexity would increase if this
    driver had to check whether speed is handled properly for each card init.
  */
  sdio_ck_freq_set(SDIO_CK_FREQ_24MHZ);

  // test communication still works
  status =
      sdio_send_cmd(CMD13, card.rca << 16, SDIO_RESP_TYPE_SHORT, &short_resp);
  if (status != SDIO_OK || (((short_resp >> 9) & 0xF) != CARD_TRANSFER_MODE)) {
    return CARD_ERR_FAILED_SPEED_INCREASE_CONFIRMATION;
  }

  return CARD_OK;
}

const sd_card_info_t *sd_card_get_info(void) { return &card; }

static inline DMA_stream_t *get_dma_stream(void) { return &DMA2->STREAM[6]; }

static inline void dma_stream_disable(DMA_stream_t *stream) {
  // write is safe, won't update until all current xfers finish
  stream->CR &= ~(DMA_SxCR_EN);
  while (stream->CR & DMA_SxCR_EN)
    ;
}

static inline void dma_stream_enable(DMA_stream_t *stream) {
  stream->CR |= DMA_SxCR_EN;
  while (!(stream->CR & DMA_SxCR_EN))
    ;
}

/*

RM0390 pg. 976 DMA config
1. enable DMA2, clear any pending interrupts
2. choice between DMA2_Stream3 (or 6) channel 4 for memory location and
DMA2_stream3 (or 6) for channel 4 destination
3. dma2_stream3 channel 4 control reg
4. dma2_stream3 channel 4 select peripheral as flow controller
5. configure incremental burst transfer to 4 beats (at least from peripherlal
side)
6. enable dma2_stream3 channel 4
Note 1: must use DMA in periph flow controller mode.
Note 2: sdio generates only DMA burst requests. DMA must be configured in
incremental burst mode on periph side

pg. 215 FIFO burst config combos

RM0390 pg. 975, READ with DMA
1. set SDIO data length register
2. program the DMA channel
3. program SDIO DCTL, DTEN with 1, DTDIR with 1, DTMODE 0, DMAEN 1, DBLOCKSIZE
with 0x9 (512 bytes)
4. SDIO arg register with address location of data on card
5. SDIO command register CMD17, wait for resp ('1'), CPSMEN.
6. wait for CMDREND[6] on STA
7. wait for DBCKEND[10] on STA
8. wait until FIFO empty, RXOVERR[5]
*/
card_result_t sd_card_read_sector(uint32_t sector_num,
                                  uint8_t buff[BLOCK_SIZE_IN_BYTES]) {
  if (sector_num >= card.sector_count)
    return CARD_ERR_SECTOR_OUT_OF_BOUNDS;

  RCC->AHB1ENR |= RCC_AHB1ENR_DMA2;
  DMA2->HIFCR = DMA_HIFCR_STREAM_6_ALL_FLAGS;
  // reset SDIO data path
  SDIO->DCTRL = 0;
  SDIO->DLEN = 0;
  SDIO->ICR = 0xFFFFFFFF; // clear all SDIO status flags
  delay_ms(1);
  DMA_stream_t *stream = get_dma_stream();
  dma_stream_disable(stream);

  stream->PAR = (uint32_t)&SDIO->FIFO;
  stream->M0AR = (uint32_t)buff;
  stream->NDTR = 128; // 512 bytes block size / 4 bytes per word = 128

  stream->CR =
      (DMA_SxCR_CHSEL_4 | DMA_SxCR_DIR_PERIPH_TO_MEM | DMA_SxCR_PBURST_4_BEATS |
       DMA_SxCR_MBURST_4_BEATS | DMA_SxCR_PSIZE_WORD | DMA_SxCR_MSIZE_WORD |
       DMA_SxCR_INCR_MEM | DMA_SxCR_PERIPH_CONTROLS_FLOW);

  stream->FCR = (DMA_SxFCR_DMDIS | DMA_SxFCR_FIFO_THRESHOLD_FULL);

  dma_stream_enable(stream);

  SDIO->DLEN = 512;
  SDIO->DCTRL = (SDIO_DCTRL_DTEN | SDIO_DCTRL_DTDIR_FROM_CARD_TO_CONTROLLER |
                 SDIO_DCTRL_DMAEN | SDIO_DCTRL_DBLOCKSIZE_512);

  sdio_status_t status;
  uint32_t resp;
  status = sdio_send_cmd(CMD17, sector_num, SDIO_RESP_TYPE_SHORT, &resp);
  if (status != SDIO_OK) {
    return CARD_ERR_FAILED_READ;
  }

  SDIO_t *sdio = SDIO;
  (void)*sdio;
  while (1) {
    // success
    if (DMA2->HISR & DMA_HISR_STREAM_6_TCIF)
      break;

    // DMA error flags for stream 6 (HISR bits 16-21)
    // bit 18 = TEIF6 (transfer error), bit 19 = DMEIF6 (direct mode error)
    // if (dma_hisr & (BIT(18) | BIT(19))) {
    //  // DMA error — capture and break
    //  break;
    //}

    //// SDIO errors
    // if (sdio_sta & (SDIO_STA_RXOVERR | SDIO_STA_DTIMEOUT |
    // SDIO_STA_DCRCFAIL)) {
    //   // SDIO data error — capture and break
    //   break;
    // }
  }

  return CARD_OK;
}

/*
  Helpers
*/
static uint64_t extract_bits_from_raw_long(const uint32_t raw[4], uint8_t msb,
                                           uint8_t lsb) {

  uint8_t width = msb - lsb + 1;

  if (width > 64 || msb < lsb || msb > 127 || lsb < 0)
    return 0;

  // the raw response is 4 consecutive 32 bit numbers that represent
  // the MSB -> LSB of a 128 bit card register.
  // pulling the product name is 5 chars X 8 bits = 40 bits, so result size
  // must be minimum uint64_t
  uint64_t result = 0;

  const uint8_t *bytes = (const uint8_t *)raw;

  // iterate from MSB down to LSB
  for (int i = 0; i < width; i++) {
    int curr_bit = msb - i;
    int byte_idx = ((127 - curr_bit) >> 3); // divide by 8
    int bit_idx = ((curr_bit) % 8);         // MSB of byte down

    uint8_t bit_val = (bytes[byte_idx] >> bit_idx) & 1;

    result = ((result << 1) | bit_val);
  }
  return result;
}

static inline bool is_high_capacity_card(uint32_t raw_resp) {
  return (raw_resp & HIGH_CAPACITY_BIT) != 0;
}

static inline void save_product_name(const uint32_t raw_resp[4]) {
  union pnm_cast {
    uint64_t as_uint64;
    char as_chars[5]; // will end up in reverse order due to little endian
                      // representation in memory
  };

  union pnm_cast pnm;
  pnm.as_uint64 =
      extract_bits_from_raw_long(raw_resp, CID_PNM_MSB, CID_PNM_LSB);

  for (int i = 0; i < 5; i++) {
    card.name[i] = pnm.as_chars[4 - i];
  }

  card.name[5] = '\0';
}

static inline void save_rca(uint32_t raw_resp) {
  // RM 0390 pg. 1000
  card.rca = ((raw_resp >> 16) & 0xFFFF);
}

static inline void save_capacity_info(const uint32_t raw_resp[4]) {
  uint64_t c_size =
      extract_bits_from_raw_long(raw_resp, CSD_C_SIZE_MSB, CSD_C_SIZE_LSB);

  // sd spec pg. 234
  card.sector_count = (c_size + 1) * 1024;
  card.capacity_bytes = (card.sector_count * BLOCK_SIZE_IN_BYTES);
}
