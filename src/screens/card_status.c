#include "assets/fonts/ibm_bios_16.h"
#include "common_colors.h"
#include "graphics/renderer.h"
#include "screens.h"
#include "storage/storage.h"

void card_status_enter(void);
screen_t *card_status_handle_event(event_t event);

screen_t card_status = {
    .enter = card_status_enter,
    .handle_event = card_status_handle_event,
};

static void bg_draw(void);
static void card_status_draw(void);

void card_status_enter(void) {
  bg_draw();
  card_status_draw();
}

screen_t *card_status_handle_event(event_t event) {
  switch (event) {
  case EVENT_STORAGE_STATE_CHANGE:
    card_status_draw();
    break;
  case EVENT_ENCODER_PRESSED:
    return &menu;
  default:
    // do nothing
    break;
  }

  return &card_status;
}

static void bg_draw(void) { renderer_fill_screen(BG_COLOR); }

static void card_status_draw(void) {
  renderer_draw_rect(0, 0, renderer_get_screen_width() - 1, 16, BG_COLOR);

  const storage_info_t *info = storage_get_info();
  char *text;

  switch (info->status) {
  case STORAGE_NO_MEDIA:
    text = "No Media";
    break;
  case STORAGE_INITIALIZING:
    text = "Initializing...";
    break;
  case STORAGE_ERROR:
    text = "STORAGE ERROR";
    break;
  case STORAGE_READY:
    text = "Storage READY!";
  }

  renderer_draw_text(0, 0, text, &ibm_bios_16, TEXT_COLOR, BG_COLOR);
}
