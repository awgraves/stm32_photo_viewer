#pragma once

typedef enum {
  /* phyical inputs */
  EVENT_ENCODER_CW,
  EVENT_ENCODER_CCW,
  EVENT_ENCODER_PRESSED,
  /* application events */
  EVENT_STORAGE_STATE_CHANGE,
} event_t;
