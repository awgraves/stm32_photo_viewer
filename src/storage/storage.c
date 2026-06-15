#include "storage.h"
#include "drivers/sd_card.h"
#include "events/event_queue.h"
#include "mcu/time.h"
#include <stdbool.h>

#define MOUNT_DELAY_MS 1000 // quick and dirty, ~1 sec at 10 ms poll interval

typedef struct {
  storage_status_t status;
  uint32_t mount_last_attempt_ms;
  bool card_inserted;
  bool card_ready;
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
    state.mount_last_attempt_ms = millis();
    break;
  case STORAGE_NO_MEDIA:
    state.card_ready = false;
    break;
  case STORAGE_READY:
    state.card_ready = true;
  default:
    break;
  }

  if (next != state.status) {
    event_queue_push(EVENT_STORAGE_STATE_CHANGE);
  }
  state.status = next;
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

  // mounting logic
  if (state.status == STORAGE_INITIALIZING) {
    if (millis() - state.mount_last_attempt_ms >= MOUNT_DELAY_MS) {
      if ((state.card_ready = sd_card_probe())) {
        status_change(STORAGE_READY);
      } else {
        status_change(STORAGE_ERROR);
      };
    }
  }
}

storage_status_t storage_get_status(void) { return state.status; };
