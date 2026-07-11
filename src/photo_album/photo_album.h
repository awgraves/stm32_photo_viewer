#pragma once
#include <stdint.h>

void photo_album_refresh(void);
uint32_t photo_album_get_photo_count(void);

void photo_album_open_curr(void);
void photo_album_open_next(void);
void photo_album_open_previous(void);
