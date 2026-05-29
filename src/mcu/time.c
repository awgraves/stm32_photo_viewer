#include "time.h"
#include "registers.h"
#include "sysclock.h"

static volatile uint32_t systicks = 0;

void time_init(void) {
  uint16_t cpu_ticks_per_ms = (sysclock_get_cpu_hz() / 1000);

  SYSTICK->RVR = cpu_ticks_per_ms - 1; // set reload value for 1 ms
  SYSTICK->CVR = 0UL;                  // clear the value register
  SYSTICK->CSR |= SYSTICK_CSR_BITS;    // config and enable
}

void Systick_Handler(void) { systicks++; }

void delay_ms(uint32_t ms) {
  uint32_t now = systicks;
  while (systicks - now < ms)
    ;
}
