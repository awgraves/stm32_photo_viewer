#include "option_row.h"
#include "graphics/renderer.h"

#define OPTION_ROW_X_OFFSET 16

void option_row_draw(option_row_params_t *p) {
  color_t row_color = p->focused ? p->text_color : p->bg_color;

  renderer_draw_rect(p->x, p->y, p->width, p->height, row_color);

  uint16_t text_y = p->y + (p->height >> 1) - (p->font->height_px >> 1);
  uint16_t text_x = p->x + OPTION_ROW_X_OFFSET;

  color_t t_color = p->focused ? p->bg_color : p->text_color;
  renderer_draw_text(text_x, text_y, p->text, p->font, t_color, row_color);
}
