#pragma once
#include <stdint.h>

#define ENTRIES_CAP 5000 // 5000 x 8 bytes = 40,000 = ~39KB out of 128KB RAM

typedef struct {
  uint32_t first_cluster;
  uint32_t size_in_bytes;
} file_t;

typedef struct {
  uint32_t count;
  file_t files[ENTRIES_CAP];
} files_list_t;

typedef enum {
  FILE_READ_OK,
  FILE_READ_ERR_NO_FILE,
  FILE_READ_IO_ERR
} file_result_t;
