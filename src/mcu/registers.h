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
} SYSTICK_t;

#define SYSTICK ((SYSTICK_t *const)SYSTICK_BASE)

// RM0390 pg. 57
#define RCC_BASE (0x40023800UL)
#define GPIOA_BASE (0x40020000UL)

// RM0390 pg. 170
typedef struct {
  volatile uint32_t _unused_1[12], AHB1ENR, _unused_2[4], APB2ENR,
      _unused_3[20];
} RCC_t;

#define RCC ((RCC_t *const)RCC_BASE)
// RM0390 pg.143
#define AHB1ENR_GPIOA (BIT(0))
// RM0390 pg. 148
#define APB2ENR_SPI1 (BIT(12))

// RM0390 pg. 191
typedef struct {
  volatile uint32_t MODER, OTYPER, OSPEEDER, PUPDR, IDR, ODR, BSRR, LCKR, AFRL,
      AFRH;
} GPIO_t;

#define GPIOA ((GPIO_t *const)GPIOA_BASE)

// SPI register map RM0390 pg. 874
typedef struct {
  volatile uint32_t CR1, CR2, SR, DR, CRCPR, RXCRCR, TXCRCR, _unused[2];
} SPI_t;

// CR1
#define SPI_ENABLE (BIT(6))
#define SPI_MASTER (BIT(2))
#define SPI_SSM (BIT(9))
#define SPI_SSI (BIT(8))

// SR
#define SPI_BSY (BIT(7))
#define SPI_TXE (BIT(1))
#define SPI_RXNE (BIT(0))

// on APB2 bus
#define SPI1_BASE (0x40013000UL)
#define SPI1 ((SPI_t *const)SPI1_BASE)
