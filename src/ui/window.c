#include "window.h"
#include "graphics/renderer.h"

void window_draw(window_params_t *p) {
  box_params_t bp = {.x = p->x,
                     .y = p->y,
                     .height = p->height,
                     .width = p->width,
                     .line_color = p->line_color,
                     .fill_color = p->fill_color,
                     .line_thickness = p->line_thickness};
  renderer_draw_box(&bp);

  uint16_t title_width = renderer_get_text_width(p->title_text, p->title_font);
  uint16_t title_x = (p->x + (p->width >> 1)) - (title_width >> 1);
  uint16_t title_y = p->y - (p->title_font->height_px >> 1);
  renderer_draw_text(title_x, title_y, p->title_text, p->title_font,
                     p->line_color, p->fill_color);
}
