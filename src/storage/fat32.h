#pragma once

typedef enum {
  FAT32_OK,
  FAT32_ERR_MBR_PARSE,
  FAT32_ERR_VBR_PARSE,
} fat32_result_t;

fat32_result_t fat32_mount(void);
