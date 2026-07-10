#include "assets/fonts/terminus_bold_16.h"
#include "common_colors.h"
#include "graphics/renderer.h"
#include "photo_album/photo_album.h"
#include "screens.h"
#include "storage/storage.h"

void viewer_enter(void);
screen_t *viewer_handle_event(event_t event);

screen_t viewer = {
    .enter = viewer_enter,
    .handle_event = viewer_handle_event,
};

static inline void draw_error(void);

#define BUFF_SIZE (64 * 1024) // 64kb out of 128kb total RAM, ie 50% of RAM
static uint8_t buff[BUFF_SIZE];

void render_curr_photo(void) {
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
    draw_error();
}

void viewer_enter(void) { render_curr_photo(); }

screen_t *viewer_handle_event(event_t event) {
  switch (event) {
  case EVENT_ENCODER_PRESSED:
    return &menu;
  case EVENT_STORAGE_STATE_CHANGE:
    return &init;
  case EVENT_ENCODER_CW:
    if (photo_album_open_next()) {
      render_curr_photo();
    };
    return &viewer;
  case EVENT_ENCODER_CCW:
    if (photo_album_open_previous()) {
      render_curr_photo();
    }
    return &viewer;
  default:
    return &viewer;
  }
}

static inline void draw_error(void) {
  renderer_fill_screen(BG_COLOR);

  const char *text = "Error reading file."; // 19 chars
  uint16_t x = renderer_get_centered_x(19 * terminus_bold_16.width_px);
  uint16_t y = renderer_get_centered_y(terminus_bold_16.height_px) -
               (DISPLAY_HEIGHT_PIXELS >> 2);

  renderer_draw_text(x, y, text, &terminus_bold_16, TEXT_COLOR, BG_COLOR);
}
