#include "../time.h"
#include "registers.h"

static volatile uint32_t systicks = 0;

void time_init(void) {
  SYSTICK->RVR = (CPU_TICKS_PER_MS)-1; // set reload value for 1 ms
  SYSTICK->CVR = 0UL;                  // clear the value register
  SYSTICK->CSR |= SYSTICK_CSR_BITS;    // config and enable
}

void Systick_Handler(void) { systicks++; }

void delay_ms(uint32_t ms) {
  uint32_t now = systicks;
  while (systicks - now < ms)
    ;
}
