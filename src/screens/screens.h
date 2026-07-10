#pragma once
#include "events/event.h"

typedef struct screen screen_t;
struct screen {
  void (*enter)(void);
  screen_t *(*handle_event)(event_t event);
};

extern screen_t init;
extern screen_t menu;
extern screen_t viewer;
