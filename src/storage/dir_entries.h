#pragma once
#include <stdint.h>

#define BUFF_ENTRIES_CAP 10

typedef struct {
  char short_name[12]; // include string null terminator
  uint32_t first_cluster;
} dir_entry_t;

typedef struct {
  uint32_t total_entries;
  dir_entry_t buffered_entries[BUFF_ENTRIES_CAP];
  uint32_t buffered_count;
} dir_entries_list_t;
