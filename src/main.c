#include "board/board.h"
#include "drivers/time.h"
#include "events/event_queue.h"
#include "inputs/poll.h"
#include "screens/screens.h"
#include "screens/splash.h"
#include "slideshow/slideshow.h"
#include "storage/storage.h"

#define POLL_MS_INTERVAL 5 // fine enough that UI lag is imperceptible

void process_events(void);
screen_t *curr_screen;

int main() {
  board_init();

  // splash_show();

  curr_screen = &init;
  curr_screen->enter();

  uint32_t last_poll = 0;
  uint32_t now = 0;
  while (1) {
    now = millis();
    if (now - last_poll >= POLL_MS_INTERVAL) {
      last_poll = now;
      storage_poll();
      inputs_poll();
      slideshow_poll();
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
      curr_screen->exit();
      curr_screen = next;
      curr_screen->enter();
    }
  }
}
