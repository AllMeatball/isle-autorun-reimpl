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
}

bool running = true;

struct Autorun_Streamer
{
    SDL_Texture *texture;
};

struct Autorun_Item
{
    int x, y;
    bool button;
    SDL_Texture *texture;
};

std::map<std::string, Autorun_Item> Autorun_items;

SDL_Window *Autorun_window;
SDL_Renderer *Autorun_renderer;

dictionary *Autorun_ini;

SDL_Texture *Autorun_backgroundTexture;


void Autorun_LoadItems() {
}

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

    SDL_CreateWindowAndRenderer(title, 640, 480, SDL_WINDOW_BORDERLESS, &Autorun_window, &Autorun_renderer);

    SDL_SetWindowIcon(Autorun_window, icon);
    SDL_DestroySurface(icon);

    Autorun_LoadItems();
    Autorun_backgroundTexture = Assets_BITMAP_TEXTURE(autorun_bmp);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    iniparser_freedict(Autorun_ini);

    std::map<std::string, Autorun_Item>::iterator iter;
    for (iter = Autorun_items.begin(); iter != Autorun_items.end(); iter++)
    {
        Autorun_Item item = iter->second;
        SDL_DestroyTexture(item.texture);
    }

    SDL_DestroyRenderer(Autorun_renderer);
    SDL_DestroyWindow(Autorun_window);
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    if (!running)
        return SDL_APP_SUCCESS;

    SDL_RenderTexture(Autorun_renderer, Autorun_backgroundTexture, NULL, NULL);

    std::map<std::string, Autorun_Item>::iterator iter;
    for (iter = Autorun_items.begin(); iter != Autorun_items.end(); iter++)
    {
        Autorun_Item item = iter->second;
        const SDL_FRect dst = {
            .x = (float)item.x,
            .y = (float)item.y,

            .w = (float)item.texture->w,
            .h = (float)item.texture->h,
        };

        SDL_RenderTexture(Autorun_renderer, item.texture, NULL, &dst);
    }

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
