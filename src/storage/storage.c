#include "storage.h"
#include "drivers/sd_card.h"
#include "drivers/time.h"
#include "events/event_queue.h"
#include "fat32.h"
#include <stdbool.h>

#define INIT_DELAY_MS 1000 // quick and dirty, ~1 sec at 10 ms poll interval
#define INIT_MAX_RETRIES 3

typedef struct {
  // init values
  uint32_t last_init_attempt_ms;
  bool card_inserted;
  bool card_ready;
  uint8_t remaining_init_retries;
  // public info
  storage_info_t info;
} state_t;

static state_t state = {0};
static void status_change(storage_status_t next);
static void check_card_insertion(void);
static void attempt_initialization(void);

/*
  Public API
*/

void storage_poll(void) {
  check_card_insertion();

  if (state.info.status == STORAGE_INITIALIZING) {
    if (millis() - state.last_init_attempt_ms >= INIT_DELAY_MS) {
      attempt_initialization();
    }
  }
}

const storage_info_t *storage_get_info(void) { return &state.info; };

void storage_open_file(const file_t *file) { fat32_open_file(file); }

file_result_t storage_read_file(uint8_t *buff, uint32_t buff_len,
                                uint32_t *bytes_read) {
  return fat32_read_file(buff, buff_len, bytes_read);
}

/*
  Helpers
*/

static void check_card_insertion(void) {
  bool inserted = sd_card_inserted();
  if (inserted && !state.card_inserted) {
    status_change(STORAGE_INITIALIZING);
  } else if (!inserted && state.card_inserted) {
    status_change(STORAGE_NO_MEDIA);
  }

  state.card_inserted = inserted;
}

static void attempt_initialization(void) {
  if (sd_card_initialize() == CARD_OK) {
    if (fat32_mount() == FAT32_OK) {
      state.info.files_list = fat32_get_files_list();
      return state.info.files_list->count > 0
                 ? status_change(STORAGE_READY)
                 : status_change(STORAGE_ERR_NO_PHOTOS);
    } else {
      status_change(STORAGE_ERR_FS_MOUNT_FAILURE);
    }
  } else if (state.remaining_init_retries > 0) {
    state.remaining_init_retries--;
    state.last_init_attempt_ms = millis();
  } else {
    status_change(STORAGE_ERR_CARD_INIT_FAILURE);
  };
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
    state.info.card = sd_card_get_info();
    break;
  default:
    break;
  }

  if (next != state.info.status) {
    event_queue_push(EVENT_STORAGE_STATE_CHANGE);
  }
  state.info.status = next;
}
