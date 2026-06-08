#include "rotary_encoder.h"
#include "input/event_queue.h"
#include "mcu/exti.h"
#include "mcu/time.h"

typedef struct {
  gpio_pin_t sw1;
  gpio_pin_t enca;
  gpio_pin_t encb;

  // switch stuff
  uint32_t last_sw_press;
  gpio_digital_t last_sw_state;
} rotary_state_t;

static rotary_state_t rot_state;

// static void handle_sw_pressed(void);
static void handle_enca_drop(void);

void rotary_encoder_init(rotary_encoder_config_t *config) {
  rot_state.sw1 = config->sw1;
  rot_state.enca = config->enca;
  rot_state.encb = config->encb;
  rot_state.last_sw_press = 0;
  rot_state.last_sw_state = HIGH;

  // set up clock and pin modes
  gpio_set_mode(config->sw1, GPIO_MODE_INPUT);
  gpio_set_mode(config->enca, GPIO_MODE_INPUT);
  gpio_set_mode(config->encb, GPIO_MODE_INPUT);

  // config pullups for all
  gpio_set_pupd(config->sw1, GPIO_PUPD_PULL_UP);
  gpio_set_pupd(config->enca, GPIO_PUPD_PULL_UP);
  gpio_set_pupd(config->encb, GPIO_PUPD_PULL_UP);

  // set up the interrupts
  // exti_config_t sw_conf = {.pin = config->sw1,
  //                         .trigger = EXTI_TRIGGER_EDGE_RISING,
  //                         .callback = &handle_sw_pressed};
  // exti_configure(&sw_conf);

  exti_config_t enca_conf = {.pin = config->enca,
                             .trigger = EXTI_TRIGGER_EDGE_FALLING,
                             .callback = &handle_enca_drop};
  exti_configure(&enca_conf);
}

void rotary_encoder_button_poll(void) {
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

// static void handle_sw_pressed(void) {
//   uint32_t now = millis();
//   if ((now - last_center_press) < 50)
//     return; // debounce
//
//   last_center_press = now;
//   event_queue_push(INPUT_EVENT_ENCODER_CW);
// }

static volatile uint32_t last_enc_update = 0;
static void handle_enca_drop(void) {
  uint32_t now = millis();
  if ((now - last_enc_update) < 100)
    return; // debounce

  last_enc_update = now;
  if (gpio_digital_read(rot_state.encb))
    event_queue_push(INPUT_EVENT_ENCODER_CW);
  else
    event_queue_push(INPUT_EVENT_ENCODER_CCW);
}
