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
