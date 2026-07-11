#include "assets/bitmaps/title_text.h"
#include "assets/fonts/ibm_bios_16.h"
#include "assets/fonts/terminus_bold_16.h"
#include "common_colors.h"
#include "graphics/renderer.h"
#include "screens.h"
#include "ui/option_row.h"
#include "ui/window.h"

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
  default:
    // do nothing
    break;
  }

  return &menu;
}

static void bg_draw(void) { renderer_fill_screen(BG_COLOR); }

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

#define NUM_TIMING_OPTS 5
static const char *ops[NUM_TIMING_OPTS] = {"Manual (using knob)", "2 seconds",
                                           "5 seconds", "10 seconds",
                                           "30 seconds"};

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
  const char *photo_count_text = "53 Photos"; // 9
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
    option_row_params_t op = {
        .x = wp.x + wp.line_thickness,
        .y = running_y,
        .height = WINDOW_ROW_HEIGHT,
        .width = wp.width - wp.line_thickness * 2,
        .text = ops[i],
        .font = &terminus_bold_16,
        .text_color = TEXT_COLOR,
        .bg_color = BG_COLOR,
        .focused = i == 4, // temp
    };
    option_row_draw(&op);
    running_y += op.height;
  }

  running_y += WINDOW_LAST_ROW_BOTTOM_MARGIN + WINDOW_LINE_THICKNESS;

  // ABOUT BUTTON runy + 58 + termboldheight * 2
  // button_params_t about_btn = {
  //    .x = DISPLAY_WIDTH_PIXELS >> 2,
  //    .y = running_y + (ibm_bios_16.height_px * 2) + 36,
  //    .height = ibm_bios_16.height_px << 1,
  //    .width = DISPLAY_WIDTH_PIXELS >> 1,
  //    .line_color = TEXT_COLOR,
  //    .fill_color = BG_COLOR,
  //    .highlighted = false,
  //    .text = "About",
  //    .font = &terminus_bold_16,
  //};
  // button_draw(&about_btn);
}
