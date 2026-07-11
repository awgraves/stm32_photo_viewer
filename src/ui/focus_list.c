#include "focus_list.h"

static void set_focus(focus_list_t *list, uint8_t new_idx);

/*
  Public API
*/

void focus_list_draw_all(focus_list_t *list) {
  for (uint8_t i = 0; i < list->count; i++)
    list->items[i].render(&list->items[i], i == list->focused_idx);
};

void focus_list_move_up(focus_list_t *list) {
  if (list->focused_idx > 0) {
    set_focus(list, list->focused_idx - 1);
  }
};

void focus_list_move_down(focus_list_t *list) {
  if (list->focused_idx < list->count - 1)
    set_focus(list, list->focused_idx + 1);
}

screen_t *focus_list_handle_press(focus_list_t *list) {
  focus_item_t *item = &list->items[list->focused_idx];
  return item->on_press(item);
}

/*
  Helpers
*/

static void set_focus(focus_list_t *list, uint8_t new_idx) {
  uint8_t old = list->focused_idx;
  list->items[old].render(&list->items[old], false);
  list->focused_idx = new_idx;
  list->items[new_idx].render(&list->items[new_idx], true);
}
