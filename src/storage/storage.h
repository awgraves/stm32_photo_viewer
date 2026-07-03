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

void storage_open_file(const dir_entry_t *entry);

typedef enum {
  STORAGE_READ_OK,
  STORAGE_READ_EOF,
  STORAGE_READ_IO_ERR
} storage_read_result_t;
storage_read_result_t storage_read_opened_file(uint8_t *buff, uint32_t len,
                                               uint32_t *bytes_read);
