#include "splash.h"
#include "assets/fonts/ibm_bios_16.h"
#include "assets/fonts/terminus_bold_16.h"
#include "assets/logos/splash_logo.h"
#include "graphics/renderer.h"
#include "mcu/time.h"
#include <stdbool.h>

#define BLINK_MS 300
#define TYPING_MS 25

#define CURSOR_HEIGHT (ibm_bios_16.height_px + 4)
#define CURSOR_WIDTH (ibm_bios_16.width_px - 2)
#define CURSOR_Y_TEXT_OFFSET 3

static inline void render_cursor(uint16_t x, uint16_t y, color_t color) {
  renderer_draw_rect(x, y, CURSOR_WIDTH, CURSOR_HEIGHT, color);
}

static void blink_cursor(uint16_t x, uint16_t y, uint8_t times, bool hold) {
  while (times-- > 0) {
    render_cursor(x, y, COLOR_SPLASH_BLACK);
    delay_ms(BLINK_MS);
    render_cursor(x, y, COLOR_WHITE);
    delay_ms(BLINK_MS);
  }

  if (hold) {
    render_cursor(x, y, COLOR_SPLASH_BLACK);
  }
}

void splash_show(void) {
  renderer_fill_screen(COLOR_WHITE);

  uint16_t logo_x = renderer_get_centered_x(splash_logo.width_px);
  uint16_t temp_y = splash_logo.height_px + (ibm_bios_16.height_px * 4);
  uint16_t logo_y = renderer_get_centered_y(temp_y);
  renderer_draw_rgb565_bitmap(logo_x, logo_y, &splash_logo);

  const char *text = "AO Embedded";
  uint16_t running_x =
      renderer_get_centered_x(ibm_bios_16.width_px * 11); // 11 chars
  uint16_t text_y = logo_y + splash_logo.height_px + 20;
  uint16_t cursor_y = text_y - CURSOR_Y_TEXT_OFFSET;

  blink_cursor(running_x, cursor_y, 2, false);

  const char *p = text;
  while (*p) {
    renderer_draw_char(running_x, text_y, *p, &ibm_bios_16, COLOR_SPLASH_BLACK,
                       COLOR_WHITE);
    running_x += ibm_bios_16.width_px;
    p++;
    delay_ms(TYPING_MS);
  }

  uint16_t copyright_x =
      renderer_get_centered_x(terminus_bold_16.width_px * 14); // 14 chars
  uint16_t copyright_y =
      renderer_get_screen_height() - (terminus_bold_16.height_px * 2);
  renderer_draw_text(copyright_x, copyright_y, "Copyright 2026",
                     &terminus_bold_16, COLOR_SPLASH_GREY, COLOR_WHITE);

  blink_cursor(running_x, cursor_y, 3, true);
}
