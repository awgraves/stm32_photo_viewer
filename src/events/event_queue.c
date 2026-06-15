#include "event_queue.h"
#include <stdint.h>

#define QUEUE_SIZE 16
#define QUEUE_MASK 15 // ie 1111

static volatile uint8_t queue[QUEUE_SIZE];
static volatile uint8_t head = 0;
static volatile uint8_t tail = 0;

void event_queue_push(event_t event) {
  uint8_t next = (head + 1) & QUEUE_MASK;
  if (next == tail) {
    return; // queue is full
  }

  queue[head] = (uint8_t)event;
  head = next;
}

bool event_queue_pop(event_t *event) {
  if (tail == head) {
    return false; // queue empty
  }

  *event = (event_t)queue[tail];
  tail = (tail + 1) & QUEUE_MASK;
  return true;
}
