#include "sd_card.h"
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

// block = sector = 512 bytes
#define BLOCK_SIZE_IN_BYTES 512ULL // sd card spec page. 234

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
#define CMD2 0x2                     // request CID (card identification data)
#define CMD3 0x3                     // request RCA (relative card address)
#define CMD9 0x9                     // request CSD (card-specific data)
#define CMD7 0x7                     // select card, place in transfer mode
#define CMD13 0xD                    // get current card state
#define CARD_TRANSFER_MODE 0x4 // 4 bit status num 12:9 in CMD13 resp, pg. 122
#define ACMD6 0x6              // change bus width to 4 bit spec pg. 52
#define ACMD6_ARG_4_BITS 0x2   // 4 bit width

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
  sdio_reset_bus_speed_and_width();

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
  if (status != SDIO_OK || short_resp != ARG_VOLTAGE_VAL) {
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
  */
  status =
      sdio_send_cmd(CMD55, card.rca << 16, SDIO_RESP_TYPE_SHORT, &short_resp);
  if (status != SDIO_OK) {
    return CARD_ERR_FAILED_BUS_WIDTH_CHANGE;
  }

  status =
      sdio_send_cmd(ACMD6, ACMD6_ARG_4_BITS, SDIO_RESP_TYPE_SHORT, &short_resp);
  if (status != SDIO_OK) {
    return CARD_ERR_FAILED_BUS_WIDTH_CHANGE;
  }
  sdio_bus_width_set(SDIO_BUS_WIDTH_4);

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
