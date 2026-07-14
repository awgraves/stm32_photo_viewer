#include "brightness_bar.h"
#include "graphics/renderer.h"
#include "utils/string.h"

#define LINE_THICKNESS 2

typedef struct {
  uint16_t x, y, height, width;
} outer_bar_dimensions_t;

static void get_percentage_text(uint8_t val, char s[5]);
static inline uint16_t get_x_gap(brightness_bar_params_t *p);
static inline outer_bar_dimensions_t
get_outer_bar_dimensions(brightness_bar_params_t *p, uint16_t x_gap,
                         uint16_t center_y);

/*
  Public API
*/

void brightness_bar_draw_values_only(brightness_bar_params_t *p) {
  color_t bg_color = p->focused ? p->primary_color : p->secondary_color;
  color_t bar_color = p->focused ? p->secondary_color : p->primary_color;

  uint16_t x_gap = get_x_gap(p);
  uint16_t center_y = p->y + (p->height >> 1);

  outer_bar_dimensions_t outer_bar =
      get_outer_bar_dimensions(p, x_gap, center_y);

  uint16_t pixels_per_perc = (outer_bar.width / 100);

  uint16_t filled_bar_width = (p->brightness_val * pixels_per_perc);
  uint16_t empty_bar_height = outer_bar.height - (LINE_THICKNESS << 1);

  // the 'filled' side can overlap with the outer boarder
  renderer_draw_rect(outer_bar.x, outer_bar.y, filled_bar_width,
                     outer_bar.height, bar_color);

  if (p->brightness_val < 100) {
    // the 'empty' side (must be skinner and shorter to maintain the 'border')
    renderer_draw_rect(outer_bar.x + filled_bar_width,
                       outer_bar.y + LINE_THICKNESS,
                       outer_bar.width - filled_bar_width - LINE_THICKNESS,
                       empty_bar_height, bg_color);
  }

  // percentage text
  char p_text[5];
  get_percentage_text(p->brightness_val, p_text);
  uint16_t p_text_width = renderer_get_text_width(p_text, p->font);
  uint16_t p_text_x = p->x + p->width - (x_gap >> 1) - (p_text_width >> 1);
  uint16_t p_text_y = center_y - (p->font->height_px >> 1);

  // clear previous text
  renderer_draw_rect(outer_bar.x + outer_bar.width, p_text_y, x_gap,
                     p->font->height_px, bg_color);

  renderer_draw_text(p_text_x, p_text_y, p_text, p->font, bar_color, bg_color);
}

void brightness_bar_draw(brightness_bar_params_t *p) {
  color_t bg_color = p->focused ? p->primary_color : p->secondary_color;
  color_t bar_color = p->focused ? p->secondary_color : p->primary_color;

  renderer_draw_rect(p->x, p->y, p->width, p->height, bg_color);

  uint16_t x_gap = get_x_gap(p);

  uint16_t center_y = p->y + (p->height >> 1);
  uint16_t icon_x = p->x + (x_gap >> 1) - (p->icon->width_px >> 1);
  uint16_t icon_y = center_y - (p->icon->height_px >> 1);

  color_palette_t icon_pal = {bar_color, bg_color};

  renderer_draw_indexed_bitmap(icon_x, icon_y, p->icon, icon_pal);

  // draw initial bar outline
  outer_bar_dimensions_t outer_bar =
      get_outer_bar_dimensions(p, x_gap, center_y);
  box_params_t bp = {
      .height = outer_bar.height,
      .width = outer_bar.width,
      .x = outer_bar.x,
      .y = outer_bar.y,
      .line_thickness = LINE_THICKNESS,
      .fill_color = bg_color,
      .line_color = bar_color,
  };
  renderer_draw_box(&bp);

  // draw bar internals and percentage text
  brightness_bar_draw_values_only(p);
}

/*
  Helpers
*/

static void get_percentage_text(uint8_t val, char s[5]) {
  // up to 4 slots '100%' + 1 for termination
  uint8_t n = itoa(val, s);
  s[n] = '%';
  s[++n] = '\0';
}

static inline uint16_t get_x_gap(brightness_bar_params_t *p) {
  return p->font->width_px * 5;
}

static inline outer_bar_dimensions_t
get_outer_bar_dimensions(brightness_bar_params_t *p, uint16_t x_gap,
                         uint16_t center_y) {
  uint16_t height = (p->height >> 1);

  return (outer_bar_dimensions_t){.height = height,
                                  .width = (p->width - (x_gap << 1)),
                                  .x = p->x + x_gap,
                                  .y = center_y - (height >> 1)};
}
