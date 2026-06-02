#include "splash.h"
#include "assets/splash_logo.h"
#include "graphics/renderer.h"
#include "mcu/time.h"
#include <stdbool.h>

#define BLINK_MS 300
#define TYPING_MS 25

#define CURSOR_HEIGHT (ibm_bios_16.height_px + 4)
#define CURSOR_WIDTH (ibm_bios_16.width_px - 2)
#define CURSOR_Y 162

static inline void render_cursor(uint16_t x, color_t color) {
  renderer_draw_rect(x, CURSOR_Y, CURSOR_WIDTH, CURSOR_HEIGHT, color);
}

static void blink_cursor(uint16_t x, uint8_t times, bool hold) {
  while (times-- > 0) {
    render_cursor(x, COLOR_SPLASH_BLACK);
    delay_ms(BLINK_MS);
    render_cursor(x, COLOR_WHITE);
    delay_ms(BLINK_MS);
  }

  if (hold) {
    render_cursor(x, COLOR_SPLASH_BLACK);
  }
}

void screens_splash_show(void) {
  renderer_fill_screen(COLOR_WHITE);

  renderer_draw_rgb565_bitmap(100, 30, &splash_logo);

  uint16_t runningX = 72;
  blink_cursor(runningX, 2, false);

  const char *text = "AO Embedded";
  const char *p = text;
  while (*p) {
    renderer_draw_char(runningX, 165, *p, &ibm_bios_16, COLOR_SPLASH_BLACK,
                       COLOR_WHITE);
    runningX += ibm_bios_16.width_px;
    p++;
    delay_ms(TYPING_MS);
  }

  renderer_draw_text(104, 205, "Copyright 2026", &terminus_bold_16,
                     COLOR_SPLASH_GREY, COLOR_WHITE);

  blink_cursor(runningX, 3, true);
}
