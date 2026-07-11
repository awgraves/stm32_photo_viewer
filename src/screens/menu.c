#include "assets/bitmaps/title_text.h"
#include "assets/fonts/ibm_bios_16.h"
#include "assets/fonts/terminus_bold_16.h"
#include "common_colors.h"
#include "graphics/renderer.h"
#include "photo_album/photo_album.h"
#include "screens.h"
#include "ui/focus_list.h"
#include "ui/option_row.h"
#include "ui/window.h"
#include "utils/string.h"

/*
  Public API
*/

static focus_list_t focus_list;

void menu_enter(void);
screen_t *menu_handle_event(event_t event);

screen_t menu = {
    .enter = menu_enter,
    .handle_event = menu_handle_event,
};

static void bg_draw(void);
static void menu_draw(void);

void menu_enter(void) {
  bg_draw();
  menu_draw();
}

screen_t *menu_handle_event(event_t event) {
  switch (event) {
  case EVENT_STORAGE_STATE_CHANGE:
    return &init;
  case EVENT_ENCODER_PRESSED:
    return focus_list_handle_press(&focus_list);
  case EVENT_ENCODER_CW:
    focus_list_move_down(&focus_list);
    break;
  case EVENT_ENCODER_CCW:
    focus_list_move_up(&focus_list);
    break;
  default:
    // do nothing
    break;
  }

  return &menu;
}

/*
  Helpers
*/

typedef struct {
  const char *label;
  uint8_t seconds;
} timing_opt_t;

#define NUM_TIMING_OPTS 5
static const timing_opt_t timing_opts[NUM_TIMING_OPTS] = {
    {"Manual (using knob)", 0},
    {"2 seconds", 2},
    {"5 seconds", 5},
    {"10 seconds", 10},
    {"30 seconds", 30}};

static focus_item_t focus_items[NUM_TIMING_OPTS];

static focus_list_t focus_list = {
    .items = focus_items,
    .count = NUM_TIMING_OPTS,
    .focused_idx = 0,
};

#define PHOTO_COUNT_TOP_MARGIN 16
#define PHOTO_COUNT_HEIGHT (terminus_bold_16.height_px)

#define WINDOW_TOP_MARGIN 50
#define WINDOW_OUTER_X_START 19
#define WINDOW_OUTER_Y_START 19
#define WINDOW_OUTER_WIDTH DISPLAY_WIDTH_PIXELS - 40 // 280
#define WINDOW_LINE_THICKNESS 2
#define WINDOW_FIRST_ROW_TOP_MARGIN 16
#define WINDOW_ROW_HEIGHT (terminus_bold_16.height_px + 8)
#define WINDOW_LAST_ROW_BOTTOM_MARGIN 10
#define WINDOW_HEIGHT                                                          \
  (WINDOW_FIRST_ROW_TOP_MARGIN + (WINDOW_ROW_HEIGHT * NUM_TIMING_OPTS) +       \
   WINDOW_LAST_ROW_BOTTOM_MARGIN + (WINDOW_LINE_THICKNESS << 1))

static void timing_option_render(const focus_item_t *self, bool focused) {
  option_row_params_t op = {
      .x = self->x,
      .y = self->y,
      .height = self->height,
      .width = self->width,
      .text = self->label,
      .font = &terminus_bold_16,
      .text_color = TEXT_COLOR,
      .bg_color = BG_COLOR,
      .focused = focused,
  };
  option_row_draw(&op);
}

static screen_t *timing_option_handle_press(const focus_item_t *self) {
  (void)self;
  return &viewer; // TODO: implement timer
}

static void load_photo_count_text(char s[]);

static void bg_draw(void) { renderer_fill_screen(BG_COLOR); }

static void menu_draw(void) {
  bg_draw();

  uint16_t total_height = title_text.height_px + PHOTO_COUNT_TOP_MARGIN +
                          PHOTO_COUNT_HEIGHT + WINDOW_TOP_MARGIN +
                          WINDOW_HEIGHT;

  uint16_t running_y = renderer_get_centered_y(total_height);

  // BITMAP title
  uint16_t title_x = renderer_get_centered_x(title_text.width_px);
  uint16_t title_y = running_y;
  color_palette_t p = {
      COLOR_BLUE_ALT,
      COLOR_WHITE,
  };

  renderer_draw_indexed_bitmap(title_x, title_y, &title_text, p);
  running_y += title_text.height_px;

  // PHOTO COUNT
  running_y += PHOTO_COUNT_TOP_MARGIN;
  char photo_count_text[32];
  load_photo_count_text(photo_count_text);

  uint16_t photo_count_text_width =
      renderer_get_text_width(photo_count_text, &ibm_bios_16);
  uint16_t photo_count_x = renderer_get_centered_x(photo_count_text_width);
  renderer_draw_text(photo_count_x, running_y, photo_count_text, &ibm_bios_16,
                     TEXT_COLOR, BG_COLOR);
  running_y += PHOTO_COUNT_HEIGHT;

  // TIMING WINDOW
  running_y += WINDOW_TOP_MARGIN;

  window_params_t wp = {
      .x = WINDOW_OUTER_X_START,
      .y = running_y,
      .height = WINDOW_HEIGHT,
      .width = WINDOW_OUTER_WIDTH,
      .title_text = " Timing ",
      .title_font = &ibm_bios_16,
      .line_color = TEXT_COLOR,
      .fill_color = BG_COLOR,
      .line_thickness = WINDOW_LINE_THICKNESS,
  };
  window_draw(&wp);

  running_y += WINDOW_LINE_THICKNESS + WINDOW_FIRST_ROW_TOP_MARGIN;

  // TIMING OPTIONS
  for (int i = 0; i < NUM_TIMING_OPTS; i++) {
    focus_item_t *opt = &focus_list.items[i];
    *opt = (focus_item_t){
        .x = wp.x + wp.line_thickness,
        .y = running_y,
        .height = WINDOW_ROW_HEIGHT,
        .width = wp.width - wp.line_thickness * 2,
        .label = timing_opts[i].label,
        .render = timing_option_render,
        .on_press = timing_option_handle_press,
    };
    running_y += opt->height;
  }

  focus_list_draw_all(&focus_list);

  running_y += WINDOW_LAST_ROW_BOTTOM_MARGIN + WINDOW_LINE_THICKNESS;
}

static void load_photo_count_text(char s[]) {
  uint32_t photo_count = photo_album_get_photo_count();

  uint8_t char_count = itoa(photo_count, s);
  char_count++; // account for \0
  char *label = " Photos";
  char c;
  while ((c = *label++)) {
    s[char_count++] = c;
  }
  s[char_count] = '\0';
}
