#include "timer.h"
#include "registers.h"
#include "sysclock.h"

/*
  Note: the firmware does not actually require 'advanced timer' functionality.
  It would have been better to use another 'general purpose' timer & avoid the
  code complexity.

  The idea to add PWM came after the PCB design was manufactured,
  and I realized too late that the LCD backlight pin assignment PA11
  meant it was tied to timer1 (advanced timer).

  This forced me to write more code here to handle both types of timers.
*/

typedef enum { TIMER_KIND_GP, TIMER_KIND_ADV } timer_kind_t;

typedef union {
  TIM_GP_t *gp;
  TIM_ADV_t *adv;
} timer_regs_u;

struct timer {
  timer_regs_u regs;
  timer_kind_t kind;
};

timer_t timer1 = {.regs.adv = TIM1, .kind = TIMER_KIND_ADV};

timer_t timer2 = {
    .regs.gp = TIM2,
    .kind = TIMER_KIND_GP,
};

static inline void timer_clock_enable(timer_t *tim) {
  if (tim == &timer1) {
    RCC->APB2ENR |= RCC_APB2ENR_TIM1;
  } else if (tim == &timer2) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM2;
  }
}

static inline TIM_COMMON_t *timer_get_common_regs(timer_t *tim) {
  return tim->kind == TIMER_KIND_GP ? &tim->regs.gp->common
                                    : &tim->regs.adv->common;
}

void timer_init_in_encoder_mode(timer_t *tim) {
  timer_clock_enable(tim);

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
  TIM_COMMON_t *common = timer_get_common_regs(tim);
  common->SMCR |= TIMx_SMCR_SMS_BOTH_EDGES_TRIGGER;
  // set 4 times sampling on inputs to filter out noise
  common->CCMR1 |= (TIMx_CCMR1_IC1F_4_SAMPLES | TIMx_CCMR1_IC2F_4_SAMPLES);
  // configure as active low
  common->CCER |= (TIMx_CCER_CC1_ACTIVE_LOW | TIMx_CCER_CC2_ACTIVE_LOW);
  // enable pin signal capturing for channels 1&2
  common->CCER |= (TIMx_CCER_CC1_EN | TIMx_CCER_CC2_EN);

  // set the counter reload value (when wrap around occurs)
  common->ARR = 0xFFFF;
  // place initial count halfway between 0 and reload
  common->CNT = 0x8888;

  // enable the counter
  common->CR1 |= TIMx_CR1_CEN;
}

void timer_init_in_pwm_mode(timer_t *tim, timer_pwm_config_t *conf) {
  timer_clock_enable(tim);

  /*
    Similarly to init logic for timer in encoder mode,
    I'm opting to hardcode the knowledge this uses channel 4 on an advanced
    timer.

    This is the only use-case for the firmware.

    Same tradeoff: gaining simplicity at the cost of flexibility.

    RM 0390 pg. 464 PWM mode
  */

  TIM_COMMON_t *common = timer_get_common_regs(tim);

  common->PSC = conf->psc_val;
  common->ARR = conf->arr_val; // auto reload val

  if (tim->kind == TIMER_KIND_ADV) {
    tim->regs.adv->CCR4 = conf->ccr_val;
  }

  common->CCMR2 |=
      (TIMx_CCMR2_OC4M_PWM_MODE_1 | TIMx_CCMR2_OC4PE); // CCMR2 preload enable

  common->CCER |= TIMx_CCER_CC4_EN;

  common->CR1 |= TIMx_CR1_ARPE; // auto reload preload enable

  // force the 'shadow' regs for ARR and CCR to load the preload values
  common->EGR |= TIMx_EGR_UG;

  // enable timer
  common->CR1 |= (TIMx_CR1_CEN);

  // enable main output (required for advanced timers)
  if (tim->kind == TIMER_KIND_ADV) {
    tim->regs.adv->BDTR |= TIM_ADV_BDTR_MOE;
  }
}

uint16_t timer_get_cnt(timer_t *tim) { return timer_get_common_regs(tim)->CNT; }

void timer_update_ccr(timer_t *tim, uint16_t new_ccr) {
  if (tim->kind == TIMER_KIND_ADV) {
    tim->regs.adv->CCR4 = new_ccr;
  }
}
