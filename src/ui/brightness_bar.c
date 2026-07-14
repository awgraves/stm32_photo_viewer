#include "brightness_bar.h"
#include "graphics/renderer.h"

#define LINE_THICKNESS 2

void brightness_bar_draw(brightness_bar_params_t *p) {
  color_t bg_color = p->focused ? p->primary_color : p->secondary_color;
  color_t bar_color = p->focused ? p->secondary_color : p->primary_color;

  renderer_draw_rect(p->x, p->y, p->width, p->height, bg_color);

  uint16_t x_gap = p->font->width_px * 5;

  uint16_t center_y = p->y + (p->height >> 1);
  uint16_t icon_x = p->x + (x_gap >> 1) - (p->icon->width_px >> 1);
  uint16_t icon_y = center_y - (p->icon->height_px >> 1);

  color_palette_t icon_pal = {bar_color, bg_color};

  renderer_draw_indexed_bitmap(icon_x, icon_y, p->icon, icon_pal);

  // bar itself
  uint16_t outer_bar_height = (p->height >> 1);
  uint16_t outer_bar_width = p->width - (x_gap << 1);
  uint16_t outer_bar_x = p->x + x_gap;
  uint16_t outer_bar_y = (center_y - (outer_bar_height >> 1));
  box_params_t bp = {
      .height = outer_bar_height,
      .width = outer_bar_width,
      .x = outer_bar_x,
      .y = outer_bar_y,
      .line_thickness = LINE_THICKNESS,
      .fill_color = bg_color,
      .line_color = bar_color,
  };
  renderer_draw_box(&bp);

  uint16_t pixels_per_perc = outer_bar_width / 100;

  renderer_draw_rect(outer_bar_x, outer_bar_y, 50 * pixels_per_perc,
                     outer_bar_height, bar_color);

  // percentage
  const char *p_text = "50%";
  uint16_t p_text_width = renderer_get_text_width(p_text, p->font);
  uint16_t p_text_x = p->x + p->width - (x_gap >> 1) - (p_text_width >> 1);
  uint16_t p_text_y = center_y - (p->font->height_px >> 1);

  renderer_draw_text(p_text_x, p_text_y, p_text, p->font, bar_color, bg_color);
}
