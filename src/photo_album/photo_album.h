#pragma once
#include "storage/storage.h"
#include <stdbool.h>
#include <stdint.h>

bool photo_album_refresh(void);

const files_list_t *photo_album_get_photo_list(void);

void photo_album_open_by_idx(uint32_t idx);
bool photo_album_open_next(void);
bool photo_album_open_previous(void);
