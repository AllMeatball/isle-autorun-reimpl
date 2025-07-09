#ifndef _ASSETS_H

#include <stdio.h>
#include <SDL3/SDL.h>

#define Assets_LOAD_BITMAP(name) Res_LoadBitmapFromBuffer(name, name ## _len)
SDL_Surface *Res_LoadBitmapFromBuffer(const void *data, size_t size);

#endif
