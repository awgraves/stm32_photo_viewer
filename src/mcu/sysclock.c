#include "sysclock.h"
#include "registers.h"

static uint32_t cpu_hz;

/*
See RM0390 pg 118 clock diagram
*/
void sysclock_init(cpu_freq_t freq) {
  cpu_hz = freq;

  if (freq == CPU_FREQ_16_MHZ)
    // default HW setting on board reset
    return;

  /*
    32MHZ CPU freq
  */

  /*
   RM0393 pg. 66 - if clock speed is >30mhz on 3.3v range,
   must add wait state (latency) to flash access.
   otherwise cpu outruns the speed that flash data can transfer.

   1WS is required for 30mhz - 60mhz range

   pg. 67, increasing CPU freq sequence of steps:
   1. program new latency first in FLASH_ACR
   2. check FLASH_ACR to ensure new latency has taken effect
   3. modify the CPU clock source in RCC_CFGR register
   4. check the RCC_CFGR register for clock src status SWS bits

   Also, instruction prefetch is useful "if at least 1 wait state is needed".
   enable the PRFTEN setting in FLASH_ACR

   pg. 69 instruction cache memory enabled with ICEN in FLASH_ACR
  */

  FLASH->ACR = (FLASH_ACR_LATENCY_1 | FLASH_ACR_PRFTEN | FLASH_ACR_ICEN);
  while (!(FLASH->ACR & FLASH_ACR_LATENCY_1))
    ;

  /*
      Configure PLL to output 32MHZ
  */
  RCC->CR |= RCC_CR_HSION;
  while (!(RCC->CR & RCC_CR_HSIRDY))
    ;

  /*
    pg. 129-130
    Using HSI as raw input source (16Mhz)

    PLLM: must divide input src (HSI) to arrive at a VCO input between 1-2mhz.
    Going with 16 as value, so VCO = HSI/PLLM = 1mhz.

    PLLN: multiplication factor applied to VCO input.
    Output result must be between 100mhz and 432mhz
    Going with 128, so VCO becomes 128Mhz

    PLLP: divisor of the VCO, options are limited to 2, 4, 6, 8
    Output result must not exceed 180Mhz
    Going with 4 so that 128 / 4 = 32 mhz (target)

    VCO output clock = PLL clock input X (PLLN / PLLM)
    general clock output (aka sysclock) = VCO clock / PLLP
    usb otg fs, sdio (not used, don't care right now) = VCO clock / PLLQ
  */
  // default source is set to HSI (PLLSRC bit set to 0)
  RCC->PLLCFGR = (RCC_PLLCFGR_PLLM(16) | RCC_PLLCFGR_PLLN(128) |
                  RCC_PLLCFGR_PLLP(RCC_PLLP_DIV_4));

  RCC->CR |= RCC_CR_PLLON;
  while (!(RCC->CR & RCC_CR_PLLRDY))
    ;

  // RMO390 pg. 133
  // switch over to 32MHZ PLL as sysclock source
  RCC->CFGR |= RCC_CFGR_SW(RCC_SRC_PLL);
  while (!(RCC->CFGR & RCC_CFGR_SW(RCC_SRC_PLL)))
    ;
}

uint32_t sysclock_get_cpu_hz(void) { return cpu_hz; }
