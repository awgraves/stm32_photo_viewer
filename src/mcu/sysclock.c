#include "sysclock.h"
#include "registers.h"

static uint32_t cpu_hz;

// see comment inside sysclock_init for more details on these values
typedef struct {
  uint32_t plln;
  uint32_t pllp; // for sysclock
  uint32_t pllq; // for pll48clk (sdio)
  uint16_t flash_acr_latency;
} config_t;

const config_t mhz16 = {
    .flash_acr_latency = FLASH_ACR_LATENCY_0,
    .plln = RCC_PLLCFGR_PLLN(128),            // VCO becomes 128Mhz
    .pllp = RCC_PLLCFGR_PLLP(RCC_PLLP_DIV_8), // 128 / 8 = 16mhz
    .pllq =
        RCC_PLLCFGR_PLLQ(4) // 128 / 4 = 32mhz (less than 48mhz, but acceptable)
};

const config_t mhz32 = {
    .flash_acr_latency = FLASH_ACR_LATENCY_1,
    .plln = RCC_PLLCFGR_PLLN(192),            // VCO becomes 192Mhz
    .pllp = RCC_PLLCFGR_PLLP(RCC_PLLP_DIV_6), // 192 / 6 = 32mhz
    .pllq = RCC_PLLCFGR_PLLQ(4)               // 192 / 4 = 48mhz
};

const config_t mhz40 = {
    .flash_acr_latency = FLASH_ACR_LATENCY_1,
    .plln = RCC_PLLCFGR_PLLN(240),            // VCO becomes 240Mhz
    .pllp = RCC_PLLCFGR_PLLP(RCC_PLLP_DIV_6), // 240 / 6 = 40mhz
    .pllq = RCC_PLLCFGR_PLLQ(5),              // 240 / 5 = 48mhz
};

const config_t mhz48 = {
    .flash_acr_latency = FLASH_ACR_LATENCY_1,
    .plln = RCC_PLLCFGR_PLLN(192),            // VCO becomes 192Mhz
    .pllp = RCC_PLLCFGR_PLLP(RCC_PLLP_DIV_4), // 192 / 4 = 48mhz
    .pllq = RCC_PLLCFGR_PLLQ(4)               // 192 / 4 = 48mhz
};

const config_t mhz56 = {
    .flash_acr_latency = FLASH_ACR_LATENCY_1,
    .plln = RCC_PLLCFGR_PLLN(336),            // VCO becomes 336Mhz
    .pllp = RCC_PLLCFGR_PLLP(RCC_PLLP_DIV_6), // 336 / 6 = 56mhz
    .pllq = RCC_PLLCFGR_PLLQ(7)               // 336 / 7 = 48mhz
};

const config_t mhz60 = {
    .flash_acr_latency = FLASH_ACR_LATENCY_1,
    .plln = RCC_PLLCFGR_PLLN(240),            // VCO becomes 240Mhz
    .pllp = RCC_PLLCFGR_PLLP(RCC_PLLP_DIV_4), // 240 / 4 = 60mhz
    .pllq = RCC_PLLCFGR_PLLQ(5)               // 240 / 5 = 48mhz
};

const config_t mhz84 = {
    .flash_acr_latency = FLASH_ACR_LATENCY_2,
    .plln = RCC_PLLCFGR_PLLN(336),            // VCO becomes 336Mhz
    .pllp = RCC_PLLCFGR_PLLP(RCC_PLLP_DIV_6), // 336 / 6 = 86mhz
    .pllq = RCC_PLLCFGR_PLLQ(7)               // 336 / 7 = 48mhz
};

const config_t mhz96 = {
    .flash_acr_latency = FLASH_ACR_LATENCY_3,
    .plln = RCC_PLLCFGR_PLLN(192),            // VCO becomes 192Mhz
    .pllp = RCC_PLLCFGR_PLLP(RCC_PLLP_DIV_2), // 192 / 2 = 96mhz
    .pllq = RCC_PLLCFGR_PLLQ(4)               // 192 / 4 = 48mhz
};

const config_t mhz168 = {
    .flash_acr_latency = FLASH_ACR_LATENCY_5,
    .plln = RCC_PLLCFGR_PLLN(336),            // VCO becomes 336Mhz
    .pllp = RCC_PLLCFGR_PLLP(RCC_PLLP_DIV_2), // 336 / 2 = 168mhz
    .pllq = RCC_PLLCFGR_PLLQ(7)               // 336 / 7 = 48mhz
};

/*
See RM0390 pg 118 clock diagram
*/
void sysclock_init(cpu_freq_t freq) {
  cpu_hz = freq;

  config_t const *config;
  switch (freq) {
  case CPU_FREQ_16_MHZ:
    config = &mhz16;
    break;
  case CPU_FREQ_32_MHZ:
    config = &mhz32;
    break;
  case CPU_FREQ_40_MHZ:
    config = &mhz40;
    break;
  case CPU_FREQ_48_MHZ:
    config = &mhz48;
    break;
  case CPU_FREQ_56_MHZ:
    config = &mhz56;
    break;
  case CPU_FREQ_60_MHZ:
    config = &mhz60;
    break;
  case CPU_FREQ_84_MHZ:
    config = &mhz84;
    break;
  case CPU_FREQ_96_MHZ:
    config = &mhz96;
    break;
  case CPU_FREQ_168_MHZ:
    config = &mhz168;
    break;
  }

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

  if (config->flash_acr_latency > 0) {
    FLASH->ACR =
        (config->flash_acr_latency | FLASH_ACR_PRFTEN | FLASH_ACR_ICEN);
    while (!(FLASH->ACR & FLASH_ACR_LATENCY_1))
      ;
  }

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

    PLLP: divisor of the VCO, options are limited to 2, 4, 6, 8
    Output result must not exceed 180Mhz

    PLLQ: divisor of VCO that should produce a final result of 48mhz.
    Must be between 2 and 15 (inclusive)

    VCO output clock = PLL clock input X (PLLN / PLLM)
    general clock output (aka sysclock) = VCO clock / PLLP
    usb otg fs, sdio (should equal 48mhz) = VCO clock / PLLQ
  */
  // default source is set to HSI (PLLSRC bit set to 0)
  RCC->PLLCFGR =
      (RCC_PLLCFGR_PLLM(16) | config->plln | config->pllp | config->pllq);

  RCC->CR |= RCC_CR_PLLON;
  while (!(RCC->CR & RCC_CR_PLLRDY))
    ;

  // RMO390 pg. 133
  // switch from HSI to PLL as sysclock source
  RCC->CFGR |= RCC_CFGR_SW(RCC_SRC_PLL);
  while (!(RCC->CFGR & RCC_CFGR_SWS(RCC_SRC_PLL)))
    ;
}

uint32_t sysclock_get_cpu_hz(void) { return cpu_hz; }
