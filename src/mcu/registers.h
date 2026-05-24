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

// RM0390 pg. 866
// bits 5:3 on SPI_CR1
typedef enum {
  SPI_BAUD_DIV_2,
  SPI_BAUD_DIV_4,
  SPI_BAUD_DIV_8,
  // unused
} spi_baud_t;

// CR1
#define SPI_ENABLE (BIT(6))
#define SPI_MASTER (BIT(2))
#define SPI_SSM (BIT(9))
#define SPI_SSI (BIT(8))

// SR
#define SPI_BSY (BIT(7))
#define SPI_TXE (BIT(1))

// RM0390 pg. 58
// on APB2 bus
#define SPI1_BASE (0x40013000UL)
#define SPI1 ((SPI_t *const)SPI1_BASE)

/*
For LCD
| Function  | Pin |
| --------- | --- |
| SPI1_SCK  | PA5 |
| SPI1_MOSI | PA7 |
| LCD_CS    | PA9 |
| LCD_DC    | PA8 |
| LCD_RST   | PA10 |
*/
// Nucleo board user manual pg. 47
// for nucleo board pins, see board user manual pg. 36
#define SPI1_SCK_PA_PINNO (5)
#define SPI1_MOSI_PA_PINNO (7)
#define SPI1_CS_PA_PINNO (9) // note this is manual CS
#define LCD_DC_PA_PINNO (8)
#define LCD_RST_PA_PINNO (10)

#define AF_SPI1 (0x5U)
