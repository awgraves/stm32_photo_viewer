#pragma once
#include "drivers/sd_card.h"
#include <stdbool.h>

typedef enum {
  STORAGE_NO_MEDIA,
  STORAGE_INITIALIZING,
  STORAGE_READY,
  STORAGE_ERROR
} storage_status_t;

typedef struct {
  storage_status_t status;
  const sd_card_info_t *card;
} storage_info_t;

void storage_init(void);
void storage_poll(void);
const storage_info_t *storage_get_info(void);
