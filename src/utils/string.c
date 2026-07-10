#include "string.h"

uint16_t string_len(const char *str) {
  uint16_t len = 0;
  while (*str++)
    len++;
  return len;
}
