#include "sd_card_reader.h"

static sd_card_config_t conf;

void sd_card_reader_init(sd_card_config_t *config) {
  conf = *config;
  gpio_set_mode(conf.det, GPIO_MODE_INPUT);

  sdio_init(&conf.sdio);
}

bool sd_card_inserted(void) { return gpio_digital_read(conf.det); }

bool sd_card_connect(void) {
  if (!sd_card_inserted())
    return false;

  sdio_reset();
  // some stuff here
  return true;
}
