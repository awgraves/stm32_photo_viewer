#pragma once
#include <stdint.h>

typedef enum {
  CPU_FREQ_16_MHZ = 16000000UL,
  CPU_FREQ_32_MHZ = 32000000UL,
  CPU_FREQ_40_MHZ = 40000000UL,
  CPU_FREQ_48_MHZ = 48000000UL,
  CPU_FREQ_56_MHZ = 56000000UL,
  CPU_FREQ_60_MHZ = 60000000UL,
  CPU_FREQ_84_MHZ = 84000000UL,
  CPU_FREQ_96_MHZ = 96000000UL,
  CPU_FREQ_168_MHZ =
      168000000UL // highest speed w/o overdrive config. RM0390 pg 66
} cpu_freq_t;

void sysclock_init(cpu_freq_t freq);

uint32_t sysclock_get_cpu_hz(void);
