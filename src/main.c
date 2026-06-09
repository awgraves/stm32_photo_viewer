#include "board/board.h"
#include "drivers/rotary_encoder.h"
#include "input/event_queue.h"
#include "mcu/time.h"
#include "screens/menu.h"
// #include "screens/splash.h"

#define POLL_MS_INTERVAL 10

void process_input_events(void);

int main() {
  board_init();

  // splash_show();
  menu_show();

  uint32_t last_poll = 0;
  uint32_t now = 0;

  while (1) {
    now = millis();
    if (now - last_poll >= POLL_MS_INTERVAL) {
      rotary_encoder_poll();
      last_poll = now;
    }
    process_input_events();
  }
}

void process_input_events(void) {
  input_event_t event;
  while (event_queue_pop(&event)) {
    menu_handle_event(event);
  }
}
