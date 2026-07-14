#include "assets/bitmaps/sun.h"
#include "assets/bitmaps/title_text.h"
#include "assets/fonts/ibm_bios_16.h"
#include "assets/fonts/terminus_bold_16.h"
#include "common_colors.h"
#include "drivers/display.h"
#include "graphics/renderer.h"
#include "photo_album/photo_album.h"
#include "screens.h"
#include "slideshow/slideshow.h"
#include "ui/brightness_bar.h"
#include "ui/focus_list.h"
#include "ui/option_row.h"
#include "ui/window.h"
#include "utils/string.h"

#define BRIGHTNESS_STEP_SIZE 10

static focus_list_t focus_list;
static bool nav_mode = true;

void menu_enter(void);
screen_t *menu_handle_event(event_t event);
void menu_exit(void);

screen_t menu = {
    .enter = menu_enter, .handle_event = menu_handle_event, .exit = menu_exit};

static void bg_draw(void);
static void menu_draw(void);

static void update_brightness_bar(void);

/*
  Public API
*/

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
    if (nav_mode) {
      focus_list_move_down(&focus_list);
    } else {
      display_brightness_increase(BRIGHTNESS_STEP_SIZE);
      update_brightness_bar();
    }
    break;
  case EVENT_ENCODER_CCW:
    if (nav_mode) {
      focus_list_move_up(&focus_list);
    } else {
      display_brightness_decrease(BRIGHTNESS_STEP_SIZE);
      update_brightness_bar();
    }
    break;
  default:
    // do nothing
    break;
  }

  return &menu;
}

void menu_exit(void) { return; }

/*
  Helpers
*/

typedef struct {
  const char *label;
  slideshow_mode_t slideshow_mode;
} timing_opt_t;

#define NUM_TIMING_OPTS 4
static const timing_opt_t timing_opts[NUM_TIMING_OPTS] = {
    {"Manual (using knob)", SLIDESHOW_MODE_OFF},
    {"2 seconds", SLIDESHOW_MODE_2_SECONDS},
    {"5 seconds", SLIDESHOW_MODE_5_SECONDS},
    {"10 seconds", SLIDESHOW_MODE_10_SECONDS},
};

#define NUM_FOCUS_ITEMS (NUM_TIMING_OPTS + 1) // +1 for brightness bar

static focus_item_t focus_items[NUM_FOCUS_ITEMS];

static focus_list_t focus_list = {
    .items = focus_items,
    .count = NUM_FOCUS_ITEMS,
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

#define BRIGHTNESS_TOP_MARGIN 30
#define BRIGHTNESS_HEADER_HEIGHT (ibm_bios_16.height_px)
#define BRIGHTNESS_BAR_TOP_MARGIN 8
#define BRIGHTNESS_BAR_HIGHLIGHT_HEIGHT 32
#define BRIGHTNESS_HEIGHT                                                      \
  (BRIGHTNESS_HEADER_HEIGHT + BRIGHTNESS_BAR_TOP_MARGIN +                      \
   BRIGHTNESS_BAR_HIGHLIGHT_HEIGHT)

static void timing_option_render(const focus_item_t *self, bool focused) {
  option_row_params_t op = {
      .x = self->x,
      .y = self->y,
      .height = self->height,
      .width = self->width,
      .text = self->label,
      .font = &terminus_bold_16,
      .primary_color = TEXT_COLOR,
      .secondary_color = BG_COLOR,
      .focused = focused,
  };
  option_row_draw(&op);
}

static screen_t *timing_option_handle_press(const focus_item_t *self) {
  slideshow_set_mode(*(slideshow_mode_t *)self->data);
  return &viewer;
}

static color_t bb_primary = TEXT_COLOR;
static color_t bb_secondary = BG_COLOR;

static void brightness_bar_render(const focus_item_t *self, bool focused) {
  brightness_bar_params_t bbp = {
      .x = self->x,
      .y = self->y,
      .height = self->height,
      .width = self->width,
      .brightness_val = display_brightness_get(),
      .font = &terminus_bold_16,
      .primary_color = bb_primary,
      .secondary_color = bb_secondary,
      .icon = &sun,
      .focused = focused,
  };

  brightness_bar_draw(&bbp);
}

static screen_t *brightness_bar_handle_press(const focus_item_t *self) {
  nav_mode = !nav_mode;
  if (nav_mode) {
    bb_primary = TEXT_COLOR;
    bb_secondary = BG_COLOR;
  } else {
    bb_primary = COLOR_WHITE;
    bb_secondary = COLOR_SPLASH_BLACK;
  }
  brightness_bar_render(self, true);
  return &menu;
}

static void update_brightness_bar(void) {
  brightness_bar_render(&focus_list.items[NUM_FOCUS_ITEMS - 1], true);
}

static void load_photo_count_text(char s[]);

static void bg_draw(void) { renderer_fill_screen(BG_COLOR); }

static void menu_draw(void) {
  bg_draw();

  uint16_t total_height = title_text.height_px + PHOTO_COUNT_TOP_MARGIN +
                          PHOTO_COUNT_HEIGHT + WINDOW_TOP_MARGIN +
                          WINDOW_HEIGHT + BRIGHTNESS_TOP_MARGIN +
                          BRIGHTNESS_HEIGHT;

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
        .data = (void *)&timing_opts[i].slideshow_mode,
    };
    running_y += opt->height;
  }

  running_y += WINDOW_LAST_ROW_BOTTOM_MARGIN + WINDOW_LINE_THICKNESS;
  running_y += BRIGHTNESS_TOP_MARGIN;

  // Brightness
  const char *brightness_text = "Display";
  uint16_t brightness_header_text_width =
      renderer_get_text_width(brightness_text, &ibm_bios_16);
  uint16_t brightness_header_x =
      renderer_get_centered_x(brightness_header_text_width);

  renderer_draw_text(brightness_header_x, running_y, brightness_text,
                     &ibm_bios_16, TEXT_COLOR, BG_COLOR);
  running_y += BRIGHTNESS_HEADER_HEIGHT;
  running_y += BRIGHTNESS_BAR_TOP_MARGIN;

  focus_list.items[NUM_FOCUS_ITEMS - 1] = (focus_item_t){
      .x = WINDOW_OUTER_X_START,
      .y = running_y,
      .height = BRIGHTNESS_BAR_HIGHLIGHT_HEIGHT,
      .width = WINDOW_OUTER_WIDTH,
      .render = brightness_bar_render,
      .on_press = brightness_bar_handle_press,
  };

  focus_list_draw_all(&focus_list);
}

static void load_photo_count_text(char s[]) {
  uint32_t photo_count = photo_album_get_photo_count();

  uint8_t char_count = itoa(photo_count, s);
  char *label = " Photos";
  char c;
  while ((c = *label++)) {
    s[char_count++] = c;
  }
  s[char_count] = '\0';
}
