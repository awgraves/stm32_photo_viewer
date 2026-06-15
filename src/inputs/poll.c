#include "poll.h"
#include "drivers/rotary_encoder.h"
#include "drivers/sd_card_reader.h"
#include "events/event_queue.h"

typedef struct {
  bool sd_card_connected;
  bool button_pressed;
} poll_state_t;

static poll_state_t state = {.sd_card_connected = false,
                             .button_pressed = false};

static inline void poll_sd_card_reader(void) {
  bool connected = sd_card_inserted();
  if (state.sd_card_connected && !connected) {
    event_queue_push(EVENT_SD_CARD_EJECTED);
  } else if (!state.sd_card_connected && connected) {
    event_queue_push(EVENT_SD_CARD_INSERTED);
  }
  state.sd_card_connected = connected;
}

static void poll_rotary_encoder(void) {
  rotary_state_t curr = rotary_encoder_get_state();
  if (curr.delta > 0) {
    event_queue_push(EVENT_ENCODER_CW);
  } else if (curr.delta < 0) {
    event_queue_push(EVENT_ENCODER_CCW);
  }

  if (state.button_pressed && !curr.button_pressed) {
    // activates on button release
    event_queue_push(EVENT_ENCODER_PRESSED);
  }
  state.button_pressed = curr.button_pressed;
}

void poll_inputs(void) {
  poll_sd_card_reader();
  poll_rotary_encoder();
}
