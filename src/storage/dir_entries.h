#pragma once
#include <stdint.h>

#define ENTRIES_CAP 10

typedef struct {
  char short_name[12]; // include string null terminator
  uint32_t first_cluster;
  uint32_t size_in_bytes;
} dir_entry_t;

typedef struct {
  uint32_t count;
  dir_entry_t entries[ENTRIES_CAP];
} dir_entries_list_t;
