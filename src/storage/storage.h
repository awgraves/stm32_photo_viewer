#pragma once
#include "dir_entries.h"
#include "drivers/sd_card.h"
#include <stdbool.h>

typedef enum {
  STORAGE_NO_MEDIA,
  STORAGE_INITIALIZING,
  STORAGE_READY,
  STORAGE_ERR_CARD_INIT_FAILURE,
  STORAGE_ERR_FS_MOUNT_FAILURE
} storage_status_t;

typedef struct {
  storage_status_t status;
  const sd_card_info_t *card;
  const dir_entries_list_t *dir_entries;
} storage_info_t;

void storage_poll(void);
const storage_info_t *storage_get_info(void);
