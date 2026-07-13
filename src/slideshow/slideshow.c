#include "slideshow.h"
#include "drivers/time.h"
#include "events/event_queue.h"
#include <stdint.h>

typedef struct {
  slideshow_mode_t mode;
  uint32_t last_event_ms;
  uint32_t elapsed_ms_threshold;
} state_t;

static state_t s;

void slideshow_set_mode(slideshow_mode_t mode) {
  s.mode = mode;
  s.elapsed_ms_threshold = mode * 1000; // mode vals are in seconds
  s.last_event_ms = millis();
}

slideshow_mode_t slideshow_get_mode(void) { return s.mode; }

void slideshow_poll(void) {
  if (s.mode == SLIDESHOW_MODE_OFF)
    return;
  uint32_t now = millis();
  if (now - s.last_event_ms >= s.elapsed_ms_threshold) {
    event_queue_push(EVENT_SLIDESHOW_TIMER_ELAPSED);
    s.last_event_ms = now;
  }
}
