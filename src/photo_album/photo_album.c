#include "photo_album.h"
#include "storage/storage.h"

static const files_list_t *files_list;
static uint32_t current_photo_idx = 0;

static void open_current_photo(void) {
  storage_open_file(&files_list->files[current_photo_idx]);
}

bool photo_album_refresh(void) {
  const storage_info_t *info = storage_get_info();
  if (info->status != STORAGE_READY) {
    return false;
  }

  current_photo_idx = 0;

  files_list = info->files_list;
  return true;
}

const files_list_t *photo_album_get_photo_list(void) { return files_list; }

void photo_album_open_by_idx(uint32_t idx) {
  if (idx >= files_list->count) {
    return;
  }

  current_photo_idx = idx;
  open_current_photo();
}

bool photo_album_open_next(void) {
  uint32_t prev = current_photo_idx;
  if (current_photo_idx + 1 <= files_list->count - 1) {
    current_photo_idx++;
  } else {
    current_photo_idx = 0; // wrap to beginning
  }

  open_current_photo();
  return prev != current_photo_idx;
}

bool photo_album_open_previous(void) {
  uint32_t prev = current_photo_idx;
  if (current_photo_idx >= 1) {
    current_photo_idx--;
  } else if (current_photo_idx == 0) {
    current_photo_idx = files_list->count - 1; // wrap to end
  }

  open_current_photo();
  return prev != current_photo_idx;
}
