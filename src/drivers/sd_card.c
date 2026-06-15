#include "sd_card.h"

static sd_card_config_t conf;
static sd_card_state_t state;

void sd_card_init(sd_card_config_t *config) {
  conf = *config;
  gpio_set_mode(conf.det, GPIO_MODE_INPUT);

  sdio_init(&conf.sdio);

  state.err = CARD_ERR_NONE;
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
#define ACMD41 0x29U           // assign voltage to use based on arg

bool sd_card_probe(void) {
  state.err = CARD_ERR_NONE;
  if (!sd_card_inserted()) {
    state.err = CARD_ERR_NOT_INSERTED;
    return false;
  }
  // resets both clk speed and bus width
  sdio_reset();

  // software reset
  sdio_status_t status;
  status = sdio_send_cmd(CMD0, 0, SDIO_RESP_TYPE_NONE, 0);
  if (status != SDIO_OK) {
    state.err = CARD_ERR_FAILED_RESET;
  }

  // check voltage
  uint32_t short_resp;
  status =
      sdio_send_cmd(CMD8, ARG_VOLTAGE_VAL, SDIO_RESP_TYPE_SHORT, &short_resp);
  if (status != SDIO_OK || short_resp != ARG_VOLTAGE_VAL) {
    state.err = CARD_ERR_FAILED_VOLTAGE;
    return false;
  }

  // assign voltage
  return true;
}

const sd_card_state_t *sd_card_get_state(void) { return &state; }
