#pragma once
#include <stdint.h>

typedef struct timer timer_t;

extern timer_t timer1;
extern timer_t timer2;

typedef struct {
  uint16_t psc_val; // prescaler for the clock
  uint16_t arr_val; // period
  uint16_t ccr_val; // duty cycle
} timer_pwm_config_t;

void timer_init_in_encoder_mode(timer_t *tim);

void timer_init_in_pwm_mode(timer_t *tim, timer_pwm_config_t *c);

uint16_t timer_get_cnt(timer_t *tim);
