#pragma once
#include <stdint.h>

typedef struct timer timer_t;
extern timer_t timer2;

typedef enum { TIMER_MODE_ENCODER } timer_mode_t;

typedef struct {
  timer_mode_t mode;
} timer_config_t;

void timer_init(timer_t *tim, timer_config_t *config);
uint16_t timer_get_cnt(timer_t *tim);
