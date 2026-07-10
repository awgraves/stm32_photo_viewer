#pragma once
#include "drivers/sd_card.h"
#include "files.h"
#include <stdbool.h>

typedef enum {
  STORAGE_NO_MEDIA,
  STORAGE_INITIALIZING,
  STORAGE_READY,
  STORAGE_ERR_NO_PHOTOS,
  STORAGE_ERR_CARD_INIT_FAILURE,
  STORAGE_ERR_FS_MOUNT_FAILURE
} storage_status_t;

typedef struct {
  storage_status_t status;
  const sd_card_info_t *card;
  const files_list_t *files_list;
} storage_info_t;

void storage_poll(void);
const storage_info_t *storage_get_info(void);

void storage_open_file(const file_t *file);

file_result_t storage_read_file(uint8_t *buff, uint32_t buff_len,
                                uint32_t *bytes_read);

void storage_close_file(void);
