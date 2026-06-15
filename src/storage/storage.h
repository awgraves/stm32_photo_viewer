#pragma once
#include <stdbool.h>

typedef enum {
  STORAGE_NO_MEDIA,
  STORAGE_INITIALIZING,
  STORAGE_READY,
  STORAGE_ERROR
} storage_status_t;

void storage_init(void);
void storage_poll(void);
storage_status_t storage_get_status(void);
