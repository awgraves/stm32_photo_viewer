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

#define BUFF_SIZE (20 * 1024) // 20kb out of 128kb total RAM
static uint8_t buff[BUFF_SIZE];

void viewer_enter(void) {
  uint32_t bytes_read;
  file_result_t res;
  renderer_begin_stream();
  while (1) {
    res = storage_read_file(buff, BUFF_SIZE, &bytes_read);
    if (res != FILE_READ_OK || bytes_read == 0)
      break;
    renderer_write_to_stream((uint16_t *)buff, bytes_read / 2);
  }
  renderer_end_stream();

  if (res != FILE_READ_OK)
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
