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
void Systick_Handler(void) __attribute__((weak, alias("Default_Handler")));

void EXTI0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI4_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));

// RM0390 pg. 235
__attribute__((section(".vtable"))) ISR_HANDLER_T vtable[96 + 16] = {
    (ISR_HANDLER_T)&_estack,
    &Reset_Handler,
    &NMI_Handler,
    &HardFault_Handler,
    &MemManage_Handler, // From here on below, all are DISABLED by default
    0,                  // bus fault
    0,                  // usage fault
    0,                  // reserved
    0,                  // reserved
    0,                  // reserved
    0,                  // reserved
    0,                  // SV call
    0,                  // debug monitor
    0,                  // reserved
    0,                  // PendSV
    &Systick_Handler,
    0, // WWDG
    0, // PVD,
    0, // TAMP_STAMP,
    0, // RTC_WKUP,
    0, // FLASH
    0, // RCC
    &EXTI0_IRQHandler,
    &EXTI1_IRQHandler,
    &EXTI2_IRQHandler,
    &EXTI3_IRQHandler,
    &EXTI4_IRQHandler};

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
