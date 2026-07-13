#pragma once
#include <stdint.h>

void systick_init(void);
void systick_ms(uint32_t ms);
uint32_t systick_millis(void);
