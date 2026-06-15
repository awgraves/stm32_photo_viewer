#pragma once

typedef enum {
  /* input controls */
  EVENT_ENCODER_CW,
  EVENT_ENCODER_CCW,
  EVENT_ENCODER_PRESSED,
  /* sd card */
  EVENT_SD_CARD_INSERTED,
  EVENT_SD_CARD_EJECTED,
} event_t;
