#include "time.h"
#include "mcu/systick.h"

void delay_ms(uint32_t ms) { return systick_ms(ms); }
uint32_t millis(void) { return systick_millis(); }
