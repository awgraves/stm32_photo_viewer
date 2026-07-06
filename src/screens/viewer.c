#include "assets/fonts/terminus_bold_16.h"
#include "common_colors.h"
#include "graphics/renderer.h"
#include "screens.h"
#include "storage/storage.h"

void viewer_enter(void);
screen_t *viewer_handle_event(event_t event);

screen_t viewer = {
    .enter = viewer_enter,
    .handle_event = viewer_handle_event,
};

static inline void draw_view(void);

#define BUFF_SIZE 512
static uint8_t buff[BUFF_SIZE];

void viewer_enter(void) {
  uint32_t bytes_read;
  for (int i = 0; i < BUFF_SIZE; i++)
    buff[i] = 0;
  storage_read_file(buff, BUFF_SIZE, &bytes_read);
  draw_view();
}

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
  renderer_draw_text(16, 16, (const char *)buff, &terminus_bold_16, TEXT_COLOR,
                     BG_COLOR);
}
