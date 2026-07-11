#include "photo_album.h"
#include "storage/storage.h"

static const files_list_t *files_list;
static uint32_t current_photo_idx = 0;

static void open_photo(uint32_t idx);

/*
  Public API
*/

void photo_album_refresh(void) {
  const storage_info_t *info = storage_get_info();
  files_list = info->files_list;

  current_photo_idx = 0;
}

void photo_album_open_curr(void) { open_photo(current_photo_idx); }

void photo_album_open_next(void) {
  uint32_t next = current_photo_idx + 1;
  if (next > files_list->count - 1) {
    next = 0; // wrap to beginning
  }

  open_photo(next);
}

void photo_album_open_previous(void) {
  uint32_t prev =
      current_photo_idx > 0 ? (current_photo_idx - 1) : (files_list->count - 1);
  open_photo(prev);
}

/*
  Helpers
*/

static void open_photo(uint32_t idx) {
  if (idx < 0 || idx > files_list->count - 1)
    return;

  storage_open_file(&files_list->files[idx]);
  current_photo_idx = idx;
}
