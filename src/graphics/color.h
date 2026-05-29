#pragma once
#include <stdint.h>

static inline uint16_t rgb565_swap(uint16_t raw) {
  return ((raw >> 8) | (raw << 8));
}

typedef enum {
  COLOR_RED = 0xF800U,
  COLOR_GREEN = 0x07E0U,
  COLOR_BLUE = 0x001FU
} color_t;
