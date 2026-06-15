#pragma once
#include "event.h"
#include <stdbool.h>

void event_queue_push(event_t event);
bool event_queue_pop(event_t *event);
