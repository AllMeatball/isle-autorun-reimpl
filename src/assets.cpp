// Copyright (C) 2025 AllMeatball
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "assets.h"
#include "globals.h"

SDL_Surface *Assets_LoadEmbedBitmap(const void *data, size_t size)
{
    SDL_IOStream *stream = SDL_IOFromConstMem(data, size);

    if (!stream)
    {
        SDL_Log("Failed to load bitmap stream: %s", SDL_GetError());
        return NULL;
    }

    SDL_Surface *surface = SDL_LoadBMP_IO(stream, true);
    if (!surface)
    {
        SDL_Log("Failed to load bitmap surface: %s", SDL_GetError());
        return NULL;
    }

    return surface;
}

SDL_Texture *Assets_LoadEmbedBitmapAsTexture(const void *data, size_t size)
{
    SDL_Surface *surface = Assets_LoadEmbedBitmap(data, size);
    if (!surface)
    {
        return NULL;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(Autorun_renderer, surface);
    if (!texture)
    {
        SDL_Log("Failed to load bitmap texture: %s", SDL_GetError());
        SDL_DestroySurface(surface);
        return NULL;
    }

    SDL_DestroySurface(surface);
    return texture;
}
