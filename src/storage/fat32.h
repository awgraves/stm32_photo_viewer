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

const dir_entries_list_t *fat32_get_dir_entries_list(void);

void fat32_set_open_file(uint32_t first_cluster_num);
