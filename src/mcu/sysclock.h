#pragma once
#include <stdint.h>

typedef enum {
  CPU_FREQ_16_MHZ = 16000000UL,
  CPU_FREQ_32_MHZ = 32000000UL,
  CPU_FREQ_40_MHZ = 40000000UL,
  CPU_FREQ_48_MHZ = 48000000UL
} cpu_freq_t;

void sysclock_init(cpu_freq_t freq);

uint32_t sysclock_get_cpu_hz(void);
