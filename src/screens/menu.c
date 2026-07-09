#include "assets/fonts/ibm_bios_16.h"
#include "assets/fonts/terminus_bold_16.h"
#include "common_colors.h"
#include "graphics/renderer.h"
#include "photo_album/photo_album.h"
#include "screens.h"
#include <stdbool.h>

#define WINDOW_OUTER_X_START 19
#define WINDOW_OUTER_Y_START 19
#define WINDOW_OUTER_WIDTH DISPLAY_WIDTH_PIXELS - 40 // 280
#define WINDOW_OUTER_HEIGHT (DISPLAY_HEIGHT_PIXELS - (WINDOW_OUTER_Y_START * 2))
#define WINDOW_PADDING 2
#define WINDOW_INNER_X_START (WINDOW_OUTER_X_START + WINDOW_PADDING)
#define WINDOW_INNER_Y_START (WINDOW_OUTER_Y_START + WINDOW_PADDING)
#define WINDOW_INNER_WIDTH (WINDOW_OUTER_WIDTH - (WINDOW_PADDING * 2))
#define WINDOW_INNER_HEIGHT (WINDOW_OUTER_HEIGHT - (WINDOW_PADDING * 2))

#define ROW_WIDTH WINDOW_INNER_WIDTH
#define ROW_HEIGHT 22
#define ROW_X_START WINDOW_INNER_X_START
#define ROW_Y_BASE (WINDOW_INNER_Y_START + WINDOW_OUTER_Y_START - 4)
#define ROW_TEXT_X_PADDING 8
#define ROW_TEXT_Y_PADDING 3
#define ROW_TEXT_X_START (ROW_X_START + ROW_TEXT_X_PADDING)
#define ROW_TEXT_Y_START(base) (base + ROW_TEXT_Y_PADDING)

typedef struct {
  uint16_t highlighted_idx;
  const files_list_t *files_list;
} menu_state_t;

static menu_state_t menu_state = {
    .highlighted_idx = 0,
};

void menu_enter(void);
screen_t *menu_handle_event(event_t event);

screen_t menu = {.enter = menu_enter, .handle_event = menu_handle_event};

static void menu_draw(void);
static void menu_move_up(void);
static void menu_move_down(void);

/*
 Public API
*/

void menu_enter(void) {
  photo_album_refresh();

  menu_state.files_list = photo_album_get_photo_list();

  menu_draw();
}

screen_t *menu_handle_event(event_t event) {
  switch (event) {
  case EVENT_ENCODER_CW:
    menu_move_down();
    break;
  case EVENT_ENCODER_CCW:
    menu_move_up();
    break;
  case EVENT_ENCODER_PRESSED:
    photo_album_open_by_idx(menu_state.highlighted_idx);
    return &viewer;
  case EVENT_STORAGE_STATE_CHANGE:
    return &card_status;
    break;
  default:
    break;
  }

  return &menu;
}

/*
  Helpers
*/

static inline void menu_draw_window(void) {
  renderer_fill_screen(BG_COLOR);

  renderer_draw_rect(WINDOW_OUTER_X_START, WINDOW_OUTER_Y_START,
                     WINDOW_OUTER_WIDTH, WINDOW_OUTER_HEIGHT, COLOR_WHITE);
  renderer_draw_rect(WINDOW_INNER_X_START, WINDOW_INNER_Y_START,
                     WINDOW_INNER_WIDTH, WINDOW_INNER_HEIGHT, COLOR_BLUE_ALT);

  uint16_t title_x =
      renderer_get_centered_x(ibm_bios_16.width_px * 14); // 14 chars
  renderer_draw_text(title_x, (WINDOW_OUTER_Y_START - 8), " Photo Viewer ",
                     &ibm_bios_16, TEXT_COLOR, BG_COLOR);
}

static inline void menu_draw_row(uint8_t idx) {
  color_t fg = TEXT_COLOR;
  color_t bg = BG_COLOR;

  if (idx == menu_state.highlighted_idx) {
    fg = BG_COLOR;
    bg = TEXT_COLOR;
  }

  uint16_t row_y_start = ROW_Y_BASE + (ROW_HEIGHT * idx);
  renderer_draw_rect(ROW_X_START, row_y_start, ROW_WIDTH, ROW_HEIGHT, bg);

  renderer_draw_text(ROW_TEXT_X_START, ROW_TEXT_Y_START(row_y_start),
                     menu_state.files_list->files[idx].short_name,
                     &terminus_bold_16, fg, bg);
}

static void menu_draw(void) {
  menu_draw_window();
  for (int i = 0; i < menu_state.files_list->count; i++)
    menu_draw_row(i);
}

static void menu_move_up(void) {
  if (menu_state.highlighted_idx > 0) {
    uint16_t old = menu_state.highlighted_idx;
    menu_state.highlighted_idx--;
    menu_draw_row(old);
    menu_draw_row(menu_state.highlighted_idx);
  }
}

static void menu_move_down(void) {
  if (menu_state.highlighted_idx < menu_state.files_list->count - 1) {
    uint16_t old = menu_state.highlighted_idx;
    menu_state.highlighted_idx++;
    menu_draw_row(old);
    menu_draw_row(menu_state.highlighted_idx);
  }
}
