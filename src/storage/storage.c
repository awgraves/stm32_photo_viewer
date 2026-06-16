#include "storage.h"
#include "drivers/sd_card.h"
#include "events/event_queue.h"
#include "mcu/time.h"
#include <stdbool.h>

#define INIT_DELAY_MS 1000 // quick and dirty, ~1 sec at 10 ms poll interval
#define INIT_MAX_RETRIES 3

typedef struct {
  uint32_t last_init_attempt_ms;
  bool card_inserted;
  bool card_ready;
  uint8_t remaining_init_retries;
  storage_info_t info;
} state_t;

static state_t state;
static void status_change(storage_status_t next);

void storage_init(void) {
  state.card_inserted = sd_card_inserted();
  if (state.card_inserted) {
    status_change(STORAGE_INITIALIZING);
  }
}

static void status_change(storage_status_t next) {
  switch (next) {
  case STORAGE_INITIALIZING:
    state.card_ready = false;
    state.last_init_attempt_ms = millis();
    state.remaining_init_retries = INIT_MAX_RETRIES;
    break;
  case STORAGE_NO_MEDIA:
    state.card_ready = false;
    break;
  case STORAGE_READY:
    state.card_ready = true;
  default:
    break;
  }

  if (next != state.info.status) {
    event_queue_push(EVENT_STORAGE_STATE_CHANGE);
  }
  state.info.status = next;
}

void storage_poll(void) {
  // check physical layer first
  bool inserted = sd_card_inserted();
  if (inserted && !state.card_inserted) {
    status_change(STORAGE_INITIALIZING);
  } else if (!inserted && state.card_inserted) {
    status_change(STORAGE_NO_MEDIA);
  }
  state.card_inserted = sd_card_inserted();

  if (state.info.status == STORAGE_INITIALIZING) {
    if (millis() - state.last_init_attempt_ms >= INIT_DELAY_MS) {
      if ((state.card_ready = sd_card_probe())) {
        status_change(STORAGE_READY);
      } else if (state.remaining_init_retries > 0) {
        state.remaining_init_retries--;
        state.last_init_attempt_ms = millis();
      } else {
        status_change(STORAGE_ERROR);
      };
    }
  }
}

const storage_info_t *storage_get_info(void) { return &state.info; };
