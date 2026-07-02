#include "board/board.h"
#include "events/event_queue.h"
#include "inputs/poll.h"
#include "mcu/time.h"
#include "screens/screens.h"
#include "screens/splash.h"
#include "storage/storage.h"

#define POLL_MS_INTERVAL 10

void process_events(void);
screen_t *curr_screen;

int main() {
  board_init();

  // splash_show();

  curr_screen = &card_status;
  curr_screen->enter();

  uint32_t last_poll = 0;
  uint32_t now = 0;
  while (1) {
    now = millis();
    if (now - last_poll >= POLL_MS_INTERVAL) {
      last_poll = now;
      inputs_poll();
      storage_poll();
      process_events();
    }
  }
}

void process_events(void) {
  event_t event;
  screen_t *next;
  while (event_queue_pop(&event)) {
    next = curr_screen->handle_event(event);
    if (next != curr_screen) {
      curr_screen = next;
      curr_screen->enter();
    }
  }
}
