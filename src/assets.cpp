#include "assets.h"

SDL_Surface *Res_LoadBitmapFromBuffer(const void *data, size_t size)
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
