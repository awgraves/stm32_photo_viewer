#pragma once
#include "dir_entries.h"
#include <stdbool.h>

typedef enum {
  FAT32_OK,
  FAT32_ERR_MBR_PARSE,
  FAT32_ERR_VBR_PARSE,
  FAT32_ERR_DIR_PARSE,
} fat32_result_t;

fat32_result_t fat32_mount(void);

const dir_entries_list_t *get_dir_entries_list(void);
