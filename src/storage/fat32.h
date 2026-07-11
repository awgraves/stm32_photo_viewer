#pragma once
#include "files.h"
#include <stdbool.h>

typedef enum {
  FAT32_OK,
  FAT32_ERR_MBR_PARSE,
  FAT32_ERR_VBR_PARSE,
  FAT32_ERR_DIR_PARSE,
} fat32_result_t;

fat32_result_t fat32_mount(void);

const files_list_t *fat32_get_files_list(void);

void fat32_open_file(const file_t *file);
file_result_t fat32_read_file(uint8_t *buff, uint32_t buff_len,
                              uint32_t *bytes_read);
