#include "assets/fonts/ibm_bios_16.h"
#include "common_colors.h"
#include "drivers/sd_card_reader.h"
#include "graphics/renderer.h"
#include "screens.h"

typedef struct {
  bool connected;
} state_t;

static state_t state;

void card_status_enter(void);
screen_t *card_status_handle_event(event_t event);

screen_t card_status = {
    .enter = card_status_enter,
    .handle_event = card_status_handle_event,
};

static void bg_draw(void);
static void card_status_draw(void);

void card_status_enter(void) {
  state.connected = sd_card_inserted();
  bg_draw();
  card_status_draw();
}

screen_t *card_status_handle_event(event_t event) {
  switch (event) {
  case EVENT_SD_CARD_INSERTED:
    if (!state.connected) {
      state.connected = true;
      card_status_draw();
    }
    break;
  case EVENT_SD_CARD_EJECTED:
    if (state.connected) {
      state.connected = false;
      card_status_draw();
    }
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
  const char *text = state.connected ? "Card CONNECTED" : "Card DISCONNECTED";
  renderer_draw_text(0, 0, text, &ibm_bios_16, TEXT_COLOR, BG_COLOR);
}
