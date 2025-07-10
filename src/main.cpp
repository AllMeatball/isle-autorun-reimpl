// bits of mpeg SDL stuff by PhobosLabs
// (https://github.com/phoboslab/pl_mpeg/blob/master/pl_mpeg_player_sdl.c)

#include <stdio.h>

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <map>
#include <string>

#include "assets.h"
#include "globals.h"
#include "res/bitmaps.h"

extern "C"
{
#include "iniparser.h"
#include "pl_mpeg.h"
}

bool running = true;

struct Autorun_Streamer
{
    SDL_Texture *texture;
};

struct Autorun_Item
{
    int x, y;
    SDL_Texture *texture;
};

std::map<std::string, Autorun_Item> Autorun_items;

SDL_Window *Autorun_window;
SDL_Renderer *Autorun_renderer;

dictionary *Autorun_ini;

void app_on_video(plm_t *mpeg, plm_frame_t *frame, void *user)
{
    Autorun_Streamer *self = (Autorun_Streamer *)user;

    SDL_UpdateYUVTexture(
        self->texture,
        NULL,
        frame->y.data, frame->y.width,
        frame->cb.data, frame->cb.width,
        frame->cr.data, frame->cr.width
    );
}

void app_on_audio(plm_t *mpeg, plm_samples_t *samples, void *user) {}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    SDL_Surface *icon = Assets_LOAD_BITMAP(icon_bmp);

    Autorun_ini = iniparser_load("autorun.ini");
    if (!Autorun_ini)
    {
        SDL_Log("Failed to load autorun.ini");
        return SDL_APP_FAILURE;
    }

    const char *title = iniparser_getstring(Autorun_ini, "Instance:ProgramName", "");

    SDL_CreateWindowAndRenderer(title, 640, 480, SDL_WINDOW_FULLSCREEN | SDL_WINDOW_BORDERLESS, &Autorun_window, &Autorun_renderer);

    SDL_SetWindowIcon(Autorun_window, icon);
    SDL_DestroySurface(icon);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    iniparser_freedict(Autorun_ini);

    SDL_DestroyRenderer(Autorun_renderer);
    SDL_DestroyWindow(Autorun_window);
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    if (!running)
        return SDL_APP_SUCCESS;

    SDL_RenderPresent(Autorun_renderer);
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT)
    {
        running = false;
    }

    return SDL_APP_CONTINUE;
}
