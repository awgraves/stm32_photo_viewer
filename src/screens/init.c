#include "assets/bitmaps/title_text.h"
#include "assets/fonts/ibm_bios_16.h"
#include "common_colors.h"
#include "graphics/renderer.h"
#include "photo_album/photo_album.h"
#include "screens.h"
#include "storage/storage.h"

#define TEXT_GAP_PX 32

void init_enter(void);
screen_t *init_handle_event(event_t event);
void init_exit(void);

screen_t init = {
    .enter = init_enter,
    .handle_event = init_handle_event,
    .exit = init_exit,
};

static uint16_t title_text_y = 0;
static uint16_t status_text_y = 0;

static void bg_draw(void);
static void title_text_draw(void);
static void storage_status_draw(storage_status_t status);

void init_enter(void) {
  const storage_info_t *info = storage_get_info();
  // 1 time calc
  if (title_text_y == 0) {
    title_text_y = renderer_get_centered_y(title_text.height_px +
                                           ibm_bios_16.height_px + TEXT_GAP_PX);
    status_text_y = title_text_y + title_text.height_px + TEXT_GAP_PX;
  }
  bg_draw();
  title_text_draw();
  storage_status_draw(info->status);
}

screen_t *init_handle_event(event_t event) {
  const storage_info_t *info = storage_get_info();
  switch (event) {
  case EVENT_STORAGE_STATE_CHANGE:
    storage_status_draw(info->status);
    if (info->status == STORAGE_READY) {
      photo_album_refresh();
      return &menu;
    }
    break;
  default:
    // do nothing
    break;
  }

  return &init;
}

void init_exit(void) { return; }

static void bg_draw(void) { renderer_fill_screen(BG_COLOR); }

static void title_text_draw(void) {
  uint16_t x = renderer_get_centered_x(title_text.width_px);
  const color_palette_t p = {BG_COLOR, TEXT_COLOR};
  renderer_draw_indexed_bitmap(x, title_text_y, &title_text, p);
}

static void storage_status_draw(storage_status_t status) {
  char *text;
  switch (status) {
  case STORAGE_NO_MEDIA:
    text = "Insert Card";
    break;
  case STORAGE_INITIALIZING:
    text = "Loading...";
    break;
  case STORAGE_ERR_CARD_INIT_FAILURE:
    text = "FAILED TO INIT CARD";
    break;
  case STORAGE_ERR_FS_MOUNT_FAILURE:
    text = "FAILED TO MOUNT FS";
    break;
  case STORAGE_ERR_NO_PHOTOS:
    text = "NO PHOTOS FOUND";
    break;
  case STORAGE_READY:
    // no text update
    return;
  }

  renderer_draw_rect(0, status_text_y, DISPLAY_WIDTH_PIXELS,
                     ibm_bios_16.height_px, BG_COLOR);

  uint16_t text_width = renderer_get_text_width(text, &ibm_bios_16);
  uint16_t x = renderer_get_centered_x(text_width);

  renderer_draw_text(x, status_text_y, text, &ibm_bios_16, TEXT_COLOR,
                     BG_COLOR);
}
