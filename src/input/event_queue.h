#pragma once
#include "event.h"
#include <stdbool.h>

void event_queue_push(input_event_t event);
bool event_queue_pop(input_event_t *event);
