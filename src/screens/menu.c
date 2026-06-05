#include "menu.h"
#include "assets/fonts/ibm_bios_16.h"
#include "assets/fonts/terminus_bold_16.h"
#include "graphics/renderer.h"
#include <stdbool.h>

#define TEXT_COLOR COLOR_WHITE
#define BG_COLOR COLOR_BLUE_ALT

#define WINDOW_OUTER_X_START 19
#define WINDOW_OUTER_Y_START 19
#define WINDOW_OUTER_WIDTH 280
#define WINDOW_OUTER_HEIGHT (240 - (WINDOW_OUTER_Y_START * 2))
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
  uint16_t selected_idx;
  uint16_t file_count;
  const char *filenames[];
} menu_state_t;

static menu_state_t menu_state = {
    .selected_idx = 0,
    .file_count = 8,
    .filenames = {"something.bmp", "something_else.bmp", "a_3rd_thing.bmp",
                  "how_about_another?.bmp", "more.bmp", "and_more.bmp",
                  "more_again.bmp", "eight_is_great.bmp",
                  "number_nine_number_nine.bmp", "wow_so_many_files.bmp"}};

static void menu_draw(void);

static void menu_move_up(void);
static void menu_move_down(void);

/*
 Public API
*/

void menu_show(void) {
  menu_state.selected_idx = 0;
  menu_draw();
}

void menu_handle_event(input_event_t event) {
  switch (event) {
  case INPUT_EVENT_ENCODER_CW:
    menu_move_down();
    break;
  case INPUT_EVENT_ENCODER_CCW:
    menu_move_up();
  default:
    break;
  }
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

  renderer_draw_text(47, (WINDOW_OUTER_Y_START - 8), " Image Viewer ",
                     &ibm_bios_16, TEXT_COLOR, BG_COLOR);
}

static inline void menu_draw_row(uint8_t idx) {
  color_t fg = TEXT_COLOR;
  color_t bg = BG_COLOR;

  if (idx == menu_state.selected_idx) {
    fg = BG_COLOR;
    bg = TEXT_COLOR;
  }

  uint16_t row_y_start = ROW_Y_BASE + (ROW_HEIGHT * idx);
  renderer_draw_rect(ROW_X_START, row_y_start, ROW_WIDTH, ROW_HEIGHT, bg);

  renderer_draw_text(ROW_TEXT_X_START, ROW_TEXT_Y_START(row_y_start),
                     menu_state.filenames[idx], &terminus_bold_16, fg, bg);
}

static void menu_draw(void) {
  menu_draw_window();
  for (int i = 0; i < menu_state.file_count; i++)
    menu_draw_row(i);
}

static void menu_move_up(void) {
  if (menu_state.selected_idx > 0) {
    uint16_t old = menu_state.selected_idx;
    menu_state.selected_idx--;
    menu_draw_row(old);
    menu_draw_row(menu_state.selected_idx);
  }
}

static void menu_move_down(void) {
  if (menu_state.selected_idx < menu_state.file_count - 1) {
    uint16_t old = menu_state.selected_idx;
    menu_state.selected_idx++;
    menu_draw_row(old);
    menu_draw_row(menu_state.selected_idx);
  }
}
