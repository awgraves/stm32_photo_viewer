#include "rotary_encoder.h"
#include "mcu/time.h"
#include "mcu/timer.h"

#define SWITCH_DEBOUNCE_MS 10
#define ENCODER_PULSES_PER_DETENT 4

typedef struct {
  // switch
  gpio_pin_t sw1;
  gpio_digital_t sw_state_stable;
  gpio_digital_t sw_state_last_sample;
  uint32_t sw_last_change_tick;

  // rotary
  timer_t *timer;
  uint16_t last_timer_cnt;
} io_t;

static io_t io;

void rotary_encoder_init(rotary_encoder_config_t *config) {
  io.sw1 = config->sw1;
  io.sw_state_stable = HIGH;
  io.sw_state_last_sample = HIGH;
  io.sw_last_change_tick = millis();
  io.timer = config->timer;
  io.last_timer_cnt = timer_get_cnt(config->timer);

  gpio_set_mode(config->sw1, GPIO_MODE_INPUT);
  gpio_set_mode(config->enca, GPIO_MODE_AF);
  gpio_set_mode(config->encb, GPIO_MODE_AF);

  gpio_set_AF(config->enca, config->enc_af);
  gpio_set_AF(config->encb, config->enc_af);

  gpio_set_pupd(config->sw1, GPIO_PUPD_PULL_UP);
  gpio_set_pupd(config->enca, GPIO_PUPD_PULL_UP);
  gpio_set_pupd(config->encb, GPIO_PUPD_PULL_UP);

  timer_config_t timer_conf = {.mode = TIMER_MODE_ENCODER};
  timer_init(io.timer, &timer_conf);
}

static bool rotary_encoder_button_poll(void);

rotary_state_t rotary_encoder_get_state(void) {
  uint16_t curr_cnt = timer_get_cnt(io.timer);
  int16_t raw_delta = curr_cnt - io.last_timer_cnt;

  // only register detents (full 'click') as a movement
  int16_t detents = raw_delta / ENCODER_PULSES_PER_DETENT;
  if (detents != 0) {
    io.last_timer_cnt += detents * ENCODER_PULSES_PER_DETENT;
  }

  rotary_state_t state = {.delta = detents,
                          .button_pressed = rotary_encoder_button_poll()};

  return state;
}

/*
  Helpers
*/

static bool rotary_encoder_button_poll(void) {
  gpio_digital_t curr = gpio_digital_read(io.sw1);
  uint32_t now = millis();

  if (curr != io.sw_state_last_sample) {
    io.sw_state_last_sample = curr;
    io.sw_last_change_tick = now;
  }

  if ((now - io.sw_last_change_tick) >= SWITCH_DEBOUNCE_MS) {
    io.sw_state_stable = io.sw_state_last_sample;
  }

  // active low
  return (io.sw_state_stable == LOW);
}
