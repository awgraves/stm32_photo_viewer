#include "button.h"
#include "graphics/renderer.h"

#define BUTTON_LINE_THICKNESS 2

void button_draw(button_params_t *p) {
  if (p->highlighted) {
    renderer_draw_rect(p->x, p->y, p->width, p->height, p->line_color);
  } else {
    box_params_t bp = {
        .x = p->x,
        .y = p->y,
        .height = p->height,
        .width = p->width,
        .fill_color = p->fill_color,
        .line_color = p->line_color,
        .line_thickness = BUTTON_LINE_THICKNESS,
    };
    renderer_draw_box(&bp);
  }

  uint16_t text_width = renderer_get_text_width(p->text, p->font);
  uint16_t text_x = p->x + (p->width >> 1) - (text_width >> 1);
  uint16_t text_y = p->y + (p->height >> 1) - (p->font->height_px >> 1);

  color_t fg = p->highlighted ? p->fill_color : p->line_color;
  color_t bg = p->highlighted ? p->line_color : p->fill_color;
  renderer_draw_text(text_x, text_y, p->text, p->font, fg, bg);
}
