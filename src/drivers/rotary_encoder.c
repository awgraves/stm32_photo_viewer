#include "rotary_encoder.h"
#include "input/event_queue.h"
#include "mcu/time.h"
#include "mcu/timer.h"

typedef struct {
  // switch stuff
  gpio_pin_t sw1;
  uint32_t last_sw_press;
  gpio_digital_t last_sw_state;

  // rotary stuff
  timer_t *timer;
  uint16_t last_cnt;
} rotary_state_t;

static rotary_state_t rot_state;

void rotary_encoder_init(rotary_encoder_config_t *config) {
  rot_state.sw1 = config->sw1;
  rot_state.last_sw_press = 0;
  rot_state.last_sw_state = HIGH;
  rot_state.timer = config->timer;

  gpio_set_mode(config->sw1, GPIO_MODE_INPUT);
  gpio_set_mode(config->enca, GPIO_MODE_AF);
  gpio_set_mode(config->encb, GPIO_MODE_AF);

  gpio_set_AF(config->enca, config->enc_af);
  gpio_set_AF(config->encb, config->enc_af);

  gpio_set_pupd(config->sw1, GPIO_PUPD_PULL_UP);
  gpio_set_pupd(config->enca, GPIO_PUPD_PULL_UP);
  gpio_set_pupd(config->encb, GPIO_PUPD_PULL_UP);

  timer_config_t timer_conf = {.mode = TIMER_MODE_ENCODER};
  timer_init(rot_state.timer, &timer_conf);
}

static void rotary_encoder_button_poll(void) {
  uint32_t now = millis();
  if (now - rot_state.last_sw_press < 50)
    return; // debounce

  gpio_digital_t curr = gpio_digital_read(rot_state.sw1);
  if (curr == HIGH && rot_state.last_sw_state == LOW) {
    rot_state.last_sw_press = now;
    event_queue_push(INPUT_EVENT_ENCODER_CW);
  }
  rot_state.last_sw_state = curr;
}

void rotary_encoder_poll(void) {
  uint16_t curr_cnt = timer_get_cnt(rot_state.timer);
  int16_t delta = curr_cnt - rot_state.last_cnt;
  rot_state.last_cnt = curr_cnt;

  if (delta > 0) {
    event_queue_push(INPUT_EVENT_ENCODER_CW);
  } else if (delta < 0) {
    event_queue_push(INPUT_EVENT_ENCODER_CCW);
  }

  rotary_encoder_button_poll();
}
