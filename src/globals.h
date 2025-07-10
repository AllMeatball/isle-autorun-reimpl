// Copyright (C) 2025 AllMeatball
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _GLOBALS_H
#define _GLOBALS_H
#include <SDL3/SDL.h>

extern "C" {
#include <iniparser.h>
}

extern SDL_Window *Autorun_window;
extern SDL_Renderer *Autorun_renderer;

extern dictionary *Autorun_ini;
#endif
