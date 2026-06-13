#include "card_status.h"
#include "assets/fonts/ibm_bios_16.h"
#include "common_colors.h"
#include "drivers/sd_card_reader.h"
#include "graphics/renderer.h"

typedef struct {
  bool connected;
} state_t;

static state_t state;

static void bg_draw(void);
static void card_status_draw(void);

void card_status_show(void) {
  state.connected = sd_card_inserted();
  bg_draw();
  card_status_draw();
}

void card_status_handle_event(input_event_t event) {
  switch (event) {
  case INPUT_EVENT_SD_CARD_INSERTED:
    if (!state.connected) {
      state.connected = true;
      card_status_draw();
    }
    break;
  case INPUT_EVENT_SD_CARD_EJECTED:
    if (state.connected) {
      state.connected = false;
      card_status_draw();
    }
    break;
  default:
    // do nothing
    break;
  }
}

static void bg_draw(void) { renderer_fill_screen(BG_COLOR); }

static void card_status_draw(void) {
  renderer_draw_rect(0, 0, renderer_get_screen_width() - 1, 16, BG_COLOR);
  const char *text = state.connected ? "Card CONNECTED" : "Card DISCONNECTED";
  renderer_draw_text(0, 0, text, &ibm_bios_16, TEXT_COLOR, BG_COLOR);
}
