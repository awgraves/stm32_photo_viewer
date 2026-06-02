#include "splash.h"
#include "assets/splash_logo.h"
#include "graphics/renderer.h"

void screens_splash_show(void) {
  renderer_fill_screen(COLOR_WHITE);
  // color_palette_t p = {COLOR_BLACK, COLOR_WHITE};
  renderer_draw_rgb565_bitmap(100, 30, &splash_logo);

  renderer_draw_text(72, 165, "AO Embedded", &ibm_bios_16, COLOR_BLACK,
                     COLOR_WHITE);

  renderer_draw_text(104, 205, "Copyright 2026", &terminus_bold_16, COLOR_BLACK,
                     COLOR_WHITE);
}
