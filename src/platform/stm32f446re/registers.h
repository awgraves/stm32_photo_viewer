#pragma once

#include <stdint.h>

#define BIT(n) (1U << (n))

// 16mHz
#define CPU_FREQ (16000000UL)

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
