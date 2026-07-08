#include "poll.h"
#include "drivers/rotary_encoder.h"
#include "events/event_queue.h"

typedef struct {
  bool button_pressed;
} poll_state_t;

static poll_state_t state = {.button_pressed = false};

void inputs_poll(void) {
  rotary_state_t curr = rotary_encoder_get_state();
  if (curr.delta < 0) {
    event_queue_push(EVENT_ENCODER_CW);
  } else if (curr.delta > 0) {
    event_queue_push(EVENT_ENCODER_CCW);
  }

  if (state.button_pressed && !curr.button_pressed) {
    // activates on button release
    event_queue_push(EVENT_ENCODER_PRESSED);
  }
  state.button_pressed = curr.button_pressed;
}
