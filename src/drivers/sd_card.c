#include "sd_card.h"

static sd_card_config_t conf;

void sd_card_init(sd_card_config_t *config) {
  conf = *config;
  gpio_set_mode(conf.det, GPIO_MODE_INPUT);
}

bool sd_card_check_inserted(void) { return gpio_digital_read(conf.det); }
