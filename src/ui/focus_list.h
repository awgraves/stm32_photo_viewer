#pragma once
#include "screens/screens.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct focus_item focus_item_t;

typedef void (*focus_item_render_fn)(const focus_item_t *self, bool focused);
typedef screen_t *(*focus_item_press_fn)(const focus_item_t *self);

struct focus_item {
  uint16_t x, y, width, height;
  const char *label;
  focus_item_render_fn render;
  focus_item_press_fn on_press;
  void *data;
};

typedef struct {
  focus_item_t *items;
  uint8_t count;
  uint8_t focused_idx;
} focus_list_t;

void focus_list_draw_all(focus_list_t *list);
void focus_list_move_up(focus_list_t *list);
void focus_list_move_down(focus_list_t *list);
screen_t *focus_list_handle_press(focus_list_t *list);
