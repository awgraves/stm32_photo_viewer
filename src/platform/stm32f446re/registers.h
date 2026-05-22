#pragma once

#include <stdint.h>

#define BIT(n) (1U << (n))

#define CPU_FREQ_HZ (16000000UL)
#define CPU_TICKS_PER_MS (CPU_FREQ_HZ / 1000)
/*
   Systick info comes from Arm Cortex M4 general user guid pg. 249

   For SYS_CRS values:
   bit 2 set: set clock source as internal CPU clock
   bit 1 set: enable Systick IRQ
   bit 0 set: enable the counter
*/
#define SYSTICK_BASE (0xE000E010)
#define SYSTICK_CSR_BITS (BIT(2) | BIT(1) | BIT(0))

typedef struct {
  volatile uint32_t CSR;
  volatile uint32_t RVR;
  volatile uint32_t CVR;
  volatile uint32_t CALIB;
} SYSTICK_T;

#define SYSTICK ((SYSTICK_T *const)SYSTICK_BASE)

// RM0390 pg. 57
#define RCC_BASE (0x40023800UL)
#define GPIOA_BASE (0x40020000UL)

// RM0390 pg. 170
typedef struct {
  volatile uint32_t _unused_1[12], AHB1ENR, _unused_2[25];
} RCC_T;

#define RCC ((RCC_T *const)RCC_BASE)

// RM0390 pg. 191
typedef struct {
  volatile uint32_t MODER, OTYPER, OSPEEDER, PUPDR, IDR, ODR, BSRR, LCKR, AFRL,
      AFRH;
} GPIO_T;

#define GPIOA ((GPIO_T *const)GPIOA_BASE)
