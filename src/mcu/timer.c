#include "timer.h"
#include "registers.h"

struct timer {
  TIM_t *regs;
};

timer_t timer2 = {
    .regs = TIM2,
};

static inline void timer_clock_enable(timer_t *tim) {
  if (tim == &timer2) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM2;
  }
}

void timer_init(timer_t *tim, timer_config_t *config) {
  timer_clock_enable(tim);

  switch (config->mode) {
  case TIMER_MODE_ENCODER:
    /*
    RM0390 pg. 510 general timers supports quadrature encoders

    pg. 536 Encoder interface mode
    Encoder mode acts as external clock with direction selection.
    Counter counts continuously between 0 and auto-reload value in TIMx_ARR.
    ARR must be configured before starting.

    I'm hard-coding certain settings here that work well for my EC11
   quadrature encoder. This keeps things simple for right now.
    If I needed to support additional hardware, I'd pass
    these in as flexible config values instead, at the cost of greater config
    complexity.
    */

    // Trigger counter updates on both channel A (T1) edges and channel B (T2)
    // edges
    tim->regs->SMCR |= TIMx_SMCR_SMS_BOTH_EDGES_TRIGGER;
    // set 4 times sampling on inputs to filter out noise
    tim->regs->CCMR1 |= (TIMx_CCMR1_IC1F_4_SAMPLES | TIMx_CCMR1_IC2F_4_SAMPLES);
    // configure as active low
    tim->regs->CCER |= (TIMx_CCER_CC1_ACTIVE_LOW | TIMx_CCER_CC2_ACTIVE_LOW);
    // enable pin signal capturing for channels 1&2
    tim->regs->CCER |= (TIMx_CCER_CC1_EN | TIMx_CCER_CC2_EN);

    // set the counter reload value (when wrap around occurs)
    tim->regs->ARR = 0xFFFF;
    // place initial count halfway between 0 and reload
    tim->regs->CNT = 0x8888;

    // enable the counter
    tim->regs->CR1 |= TIMx_CR1_CEN;
    break;
  default:
    break;
  }
}

uint16_t timer_get_cnt(timer_t *tim) { return tim->regs->CNT; }
