#pragma once

typedef enum {
  SLIDESHOW_MODE_OFF = 0,
  SLIDESHOW_MODE_2_SECONDS = 2,
  SLIDESHOW_MODE_5_SECONDS = 5,
  SLIDESHOW_MODE_10_SECONDS = 10,
} slideshow_mode_t;

void slideshow_set_mode(slideshow_mode_t mode);
slideshow_mode_t slideshow_get_mode(void);

void slideshow_poll(void);
