// Copyright (C) 2025 AllMeatball
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _ASSETS_H

#include <SDL3/SDL.h>
#include <stdio.h>

#define Assets_LOAD_BITMAP(name) Assets_LoadEmbedBitmap(name, name##_len)
#define Assets_BITMAP_TEXTURE(name) Assets_LoadEmbedBitmapAsTexture(name, name##_len)

SDL_Surface *Assets_LoadEmbedBitmap(const void *data, size_t size);
SDL_Texture *Assets_LoadEmbedBitmapAsTexture(const void *data, size_t size);

#endif
