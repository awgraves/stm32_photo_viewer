#include <stdint.h>

extern uint32_t _estack;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _la_data;
extern uint32_t _sbss;
extern uint32_t _ebss;

void Reset_Handler(void);
int main(void);

typedef void (*ISR_HANDLER_T)(void);

void NMI_Handler(void) __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler(void) __attribute__((weak, alias("Default_Handler")));

// RM0390 pg. 235
__attribute__((section(".vtable"))) ISR_HANDLER_T vtable[96 + 16] = {
    (ISR_HANDLER_T)&_estack, &Reset_Handler, &NMI_Handler, &HardFault_Handler,
    &MemManage_Handler};

void Default_Handler(void) {
  while (1) {
    // do nothing
  }
}

void Reset_Handler(void) {
  // copy .data section
  uint32_t *p_src = &_la_data;
  uint32_t *p_dest = &_sdata;
  while (p_dest < &_edata)
    *p_dest++ = *p_src++;

  // zero out bss section
  uint32_t *bss_p = &_sbss;
  while (bss_p < &_ebss)
    *bss_p++ = 0;

  main();

  // safety in case main exits
  while (1)
    ;
}
