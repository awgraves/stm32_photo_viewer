#pragma once

#include <stdint.h>

#define BIT(n) (1U << (n))

/*
   Systick info comes from Arm Cortex M4 general user guide pg. 249

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

// RM0390 pg. 170
typedef struct {
  volatile uint32_t CR, PLLCFGR, CFGR, _unused_1[9], AHB1ENR, _unused_2[3],
      APB1ENR, APB2ENR, _unused_3[20];
} RCC_t;

// RM0390 pg. 57
#define RCC_BASE (0x40023800UL)
#define RCC ((RCC_t *const)RCC_BASE)

// RM0390 pg. 127 - 128
#define RCC_CR_PLLRDY (BIT(25))
#define RCC_CR_PLLON (BIT(24))
#define RCC_CR_HSIRDY (BIT(1))
#define RCC_CR_HSION (BIT(0))

typedef enum { RCC_PLLP_DIV_2, RCC_PLLP_DIV_4 } PLLP_divisor_t;
#define RCC_PLLCFGR_PLLP(divisor) ((divisor & 0x1) << 16)
#define RCC_PLLCFGR_PLLN(multiplier) ((multiplier & 0x1FF) << 6)
#define RCC_PLLCFGR_PLLM(divisor) ((divisor & 0x3F) << 0)

// RM0390 pg. 132
typedef enum { RCC_SRC_HSI, RCC_SRC_HSE, RCC_SRC_PLL, RCC_SRC_PLLR } rcc_src_t;
#define RCC_CFGR_SWS(src) ((src & 0x3) << 2)
#define RCC_CFGR_SW(src) ((src & 0x3) << 0)

// RM0390 pg.143
#define RCC_AHB1ENR_GPIOA (BIT(0))
#define RCC_AHB1ENR_DMA2 (BIT(22))

// RM0390 pg. 146
#define RCC_APB1ENR_TIM2 (BIT(0))

// RM0390 pg. 148
#define RCC_APB2ENR_SPI1 (BIT(12))
#define RCC_APB2ENR_SYSCFG (BIT(14))

// RM0390 pg. 87
typedef struct {
  volatile uint32_t ACR, KEYR, OPTKEYR, SR, CR, OPTCR;
} FLASH_t;

#define FLASH_BASE (0x40023C00UL)
#define FLASH ((FLASH_t *const)FLASH_BASE)

#define FLASH_ACR_ICEN (BIT(9))
#define FLASH_ACR_PRFTEN (BIT(8))
#define FLASH_ACR_LATENCY_1 ((0x1 & 0xF) << 0)

// RM0390 pg. 191
typedef struct {
  volatile uint32_t MODER, OTYPER, OSPEEDER, PUPDR, IDR, ODR, BSRR, LCKR, AFRL,
      AFRH;
} GPIO_t;

#define GPIOA_BASE (0x40020000UL)
#define GPIOA ((GPIO_t *const)GPIOA_BASE)

// SPI register map RM0390 pg. 874
typedef struct {
  volatile uint32_t CR1, CR2, SR, DR, CRCPR, RXCRCR, TXCRCR, _unused[2];
} SPI_t;

// CR1
#define SPI_CR1_ENABLE (BIT(6))
#define SPI_CR1_MASTER (BIT(2))
#define SPI_CR1_SSM (BIT(9))
#define SPI_CR1_SSI (BIT(8))

// CR2
#define SPI_CR2_TXDMAEN (BIT(1))

// SR
#define SPI_SR_BSY (BIT(7))
#define SPI_SR_TXE (BIT(1))
#define SPI_SR_RXNE (BIT(0))

// on APB2 bus
#define SPI1_BASE (0x40013000UL)
#define SPI1 ((SPI_t *const)SPI1_BASE)

// RM0390 pg. 231
typedef struct {
  volatile uint32_t CR, NDTR, PAR, M0AR, M1AR, FCR;
} DMA_stream_t;

// RM0390 pg. 231
typedef struct {
  volatile uint32_t LISR, HISR, LIFCR, HIFCR;

  DMA_stream_t STREAM[8];
} DMA_t;

// DMA_LISR pg. 222
#define DMA_LISR_STREAM_3_TCIF (BIT(27)) // signals xfer complete

// DMA_LIFCR pg. 224
#define DMA_LIFCR_STREAM_3_ALL_FLAGS (0xFU << 24)

// DMA_SxCR pg. 225
#define DMA_SxCR_CHSEL_3 (3U << 25)
// pg. 227
#define DMA_SxCR_INCR_MEM (BIT(10))
#define DMA_SxCR_DIR_MEM_TO_PERIPH (0x01U << 6)
// pg. 228
#define DMA_SxCR_EN (BIT(0)) // cleared by hardware when DMA end of xfer

// pg. 58
#define DMA2_BASE (0x40026400)
#define DMA2 ((DMA_t *const)DMA2_BASE)

// pg. 246
typedef struct {
  volatile uint32_t IMR, EMR, RTSR, FTSR, SWIER, PR;
} EXTI_t;

// pg. 58
#define EXTI_BASE (0x40013C00UL)
#define EXTI ((EXTI_t *const)EXTI_BASE)

// pg. 199
typedef struct {
  volatile uint32_t MEMRMP, PMC, EXTICR[4], CMPCR, CFGR;
} SYSCFG_t;

// pg. 58
#define SYSCF_BASE (0x40013800UL)
#define SYSCF ((SYSCFG_t *const)SYSCF_BASE)

// pg. 195
#define SYSCFG_EXTI_INPUT_PA 0x0000U
#define SYSCFG_EXTI_INPUT_MASK(exti_pos, input) ((input) << (exti_pos * 4))

/*
ARM Cortex M4 general user manual pg. 233
*/
typedef struct {
  volatile uint32_t ISER; // 'Interrupt set-enable registers'
  // ... more registers exist, but not using them.
  /*
  Note: just need ISER 0.
  In RM 0390 pg. 236, the ISER bit to set corresponds to the 'position' col
  for EXTI0 - EXTI4, these are 6 - 10 respectively
  */
} NVIC_t;

// NOTE: this will break if pinno > 4
#define NVIC_EXTI_BIT_FROM_PINNO(pinno) (BIT(pinno + 6))

#define NVIC_BASE 0xE000E100UL
#define NVIC ((NVIC_t *const)NVIC_BASE)

// RM 0390 pg. 569
typedef struct {
  volatile uint32_t CR1, CR2, SMCR, DIER, SR, EGR, CCMR1, CCMR2, CCER, CNT, PSC,
      ARR, CCR1, CCR2, CCR3, CCR4, DCR, DMAR, OR;
} TIM_t;

// RM 0390 pg. 59, on APB1 bus
#define TIM2_BASE (0x40000000UL)
#define TIM2 ((TIM_t *const)TIM2_BASE)

// RM0390 pg. 548 (control reg1)
#define TIMx_CR1_CEN (BIT(0))
// RM0390 pg. 552 (slave mode control)
#define TIMx_SMCR_SMS_T1_EDGE (0x2U)
// RM0390 pg. 557 - 558 (capture/compare mode register)
// input capture 1 and 2 filters
#define TIMx_CCMR1_IC2F_4_SAMPLES ((0x2) << 12)
#define TIMx_CCMR1_IC1F_4_SAMPLES ((0x2) << 4)
// RM0390 pg. 562 (capture/compare enable)
#define TIMx_CCER_CC1_ACTIVE_LOW (BIT(1))
#define TIMx_CCER_CC2_ACTIVE_LOW (BIT(5))
#define TIMx_CCER_CC1_EN (BIT(0))
#define TIMx_CCER_CC2_EN (BIT(4))
