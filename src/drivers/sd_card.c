#include "sd_card.h"
#include "mcu/time.h"
#include <stdbool.h>

static sd_card_gpio_config_t conf;

typedef struct {
  bool high_capacity;
  char name[6];
} state_t;

static state_t state;

// sd spec pg. 224 outlines CID register
typedef struct __attribute__((packed)) {
  uint8_t mid;  // manufacturer id
  char oid[2];  // OEM application id, 2 printable ASCII
  char pnm[5];  // product name
  uint8_t prv;  // product revision
  uint32_t psn; // product serial number
  uint16_t mdt; // manufacturing date (lowest 12 bits only)
  uint8_t reserved;
} cid_register_t;

void sd_card_gpio_init(sd_card_gpio_config_t *config) {
  conf = *config;
  gpio_set_mode(conf.det, GPIO_MODE_INPUT);

  sdio_init(&conf.sdio);
}

bool sd_card_inserted(void) { return gpio_digital_read(conf.det); }

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
#define CMD2 0x2                     // request CID
#define CMD3 0x3                     // request RCA

static inline void save_product_name_from_cid(uint32_t raw_resp[4]);

card_result_t sd_card_initialize(void) {
  if (!sd_card_inserted()) {
    return CARD_ERR_NOT_INSERTED;
  }
  // resets both clk speed and bus width
  sdio_reset();

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

  // save capacity tier
  state.high_capacity = (short_resp & HIGH_CAPACITY_BIT) != 0;

  uint32_t long_resp[4];
  status = sdio_send_cmd(CMD2, 0, SDIO_RESP_TYPE_LONG, long_resp);
  if (status != SDIO_OK) {
    return CARD_ERR_FAILED_CID;
  }

  save_product_name_from_cid(long_resp);

  // NOTE: sd card spec pg 52 discusses bus width selection
  return CARD_OK;
}

const char *sd_card_get_name(void) { return state.name; }

/*
  Helpers
*/

static inline void save_product_name_from_cid(uint32_t raw_resp[4]) {
  uint32_t swapped[4];
  // need to swap due to endianness mismatch from SDIO registers
  swapped[0] = __builtin_bswap32(raw_resp[0]);
  swapped[1] = __builtin_bswap32(raw_resp[1]);
  swapped[2] = __builtin_bswap32(raw_resp[2]);
  swapped[3] = __builtin_bswap32(raw_resp[3]);

  cid_register_t *cid = (cid_register_t *)swapped;

  for (int i = 0; i < 5; i++) {
    state.name[i] = cid->pnm[i];
  }
  state.name[5] = '\0';
}
