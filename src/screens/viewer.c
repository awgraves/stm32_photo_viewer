#include "assets/fonts/terminus_bold_16.h"
#include "common_colors.h"
#include "graphics/renderer.h"
#include "screens.h"

void viewer_enter(void);
screen_t *viewer_handle_event(event_t event);

screen_t viewer = {
    .enter = viewer_enter,
    .handle_event = viewer_handle_event,
};

static inline void draw_view(void);

void viewer_enter(void) { draw_view(); }

screen_t *viewer_handle_event(event_t event) {
  switch (event) {
  case EVENT_ENCODER_PRESSED:
    return &menu;
  case EVENT_STORAGE_STATE_CHANGE:
    return &card_status;
  default:
    return &viewer;
  }
}

static inline void draw_view(void) {
  renderer_fill_screen(BG_COLOR);
  renderer_draw_text(
      16, 16, "File text will go inside here and should wrap around if needed.",
      &terminus_bold_16, TEXT_COLOR, BG_COLOR);
}
