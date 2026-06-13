#include "board/board.h"
#include "drivers/sd_card_reader.h"
// #include "drivers/rotary_encoder.h"
#include "input/event_queue.h"
#include "mcu/time.h"
// #include "screens/menu.h"
#include "screens/card_status.h"
#include "screens/splash.h"

#define POLL_MS_INTERVAL 10

void process_input_events(void);

int main() {
  board_init();

  splash_show();
  // menu_show();
  card_status_show();

  uint32_t last_poll = 0;
  uint32_t now = 0;
  bool last_connected = false;

  while (1) {
    now = millis();
    if (now - last_poll >= POLL_MS_INTERVAL) {
      // rotary_encoder_poll();
      bool connected = sd_card_inserted();
      if (last_connected && !connected) {
        event_queue_push(INPUT_EVENT_SD_CARD_EJECTED);
      } else if (!last_connected && connected) {
        event_queue_push(INPUT_EVENT_SD_CARD_INSERTED);
      }
      last_poll = now;
      last_connected = connected;
    }
    process_input_events();
  }
}

void process_input_events(void) {
  input_event_t event;
  while (event_queue_pop(&event)) {
    card_status_handle_event(event);
    // menu_handle_event(event);
  }
}
