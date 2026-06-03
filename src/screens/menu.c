#include "assets/fonts/ibm_bios_16.h"
#include "assets/fonts/terminus_bold_16.h"
#include "graphics/renderer.h"

#define TEXT_COLOR COLOR_WHITE
#define BG_COLOR COLOR_BLUE_ALT

#define SCAN_ROW_HEIGHT 22

#define BOX_START_X 19
#define BOX_START_Y 19
#define BOX_WIDTH 280
#define BOX_HEIGHT (240 - 1 - (BOX_START_Y * 3))
#define BOX_PADDING 2

#define TEXT_START_X (BOX_START_X + 16)
#define TEXT_START_Y (SCAN_ROW_HEIGHT * 2)

void screens_menu_show(void) {
  renderer_fill_screen(BG_COLOR);

  renderer_draw_rect(BOX_START_X, BOX_START_Y, BOX_WIDTH, BOX_HEIGHT,
                     COLOR_WHITE);
  renderer_draw_rect(BOX_START_X + BOX_PADDING, BOX_START_Y + BOX_PADDING,
                     BOX_WIDTH - (BOX_PADDING * 2),
                     BOX_HEIGHT - (BOX_PADDING * 2), COLOR_BLUE_ALT);

  renderer_draw_text(48, (SCAN_ROW_HEIGHT / 2), " Image Viewer ", &ibm_bios_16,
                     TEXT_COLOR, BG_COLOR);

  renderer_draw_text(TEXT_START_X, TEXT_START_Y, "> something.BMP",
                     &terminus_bold_16, TEXT_COLOR, BG_COLOR);
  renderer_draw_text(TEXT_START_X, TEXT_START_Y + 22, "  another_thing.BMP",
                     &terminus_bold_16, TEXT_COLOR, BG_COLOR);

  renderer_draw_text(32, BOX_START_Y + BOX_HEIGHT + 10,
                     "Scroll to move, Center to select", &terminus_bold_16,
                     TEXT_COLOR, BG_COLOR);
}
