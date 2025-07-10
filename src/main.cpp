// Copyright (C) 2025 AllMeatball
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <stdio.h>

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <map>
#include <string>

#include "assets.h"
#include "globals.h"
#include "res/bitmaps.h"
#include "utils.h"

extern "C"
{
#include "iniparser.h"
}

bool running = true;

enum Autorun_ItemType
{
    Autorun_ItemType_BUTTON,
    Autorun_ItemType_VIDEO,
};

struct Autorun_Item
{
    enum Autorun_ItemType type;

    int x, y;
    SDL_Texture *texture;

    union
    {
        struct
        {
            SDL_Texture *hover_texture;
        } button;
    };
};

std::map<std::string, Autorun_Item> Autorun_items;

SDL_Window *Autorun_window;
SDL_Renderer *Autorun_renderer;

dictionary *Autorun_ini;

SDL_Texture *Autorun_backgroundTexture;

void Autorun_AddItem(std::string name, SDL_Texture *texture, SDL_Texture *hover_texture)
{
    Autorun_Item item;
    item.type = Autorun_ItemType_BUTTON;

    item.x = (float)iniparser_getint(
        Autorun_ini,
        (name + ":PostXPos").c_str(),

        iniparser_getint(Autorun_ini, (name + ":XPos").c_str(), 0)
    );

    item.y = (float)iniparser_getint(
        Autorun_ini,
        (name + ":PostYPos").c_str(),

        iniparser_getint(Autorun_ini, (name + ":YPos").c_str(), 0)
    );

    item.texture = texture;
    item.button.hover_texture = hover_texture;

    Autorun_items[name] = item;
}

void Autorun_LoadItems()
{
    Autorun_AddItem(
        "Run",
        Assets_BITMAP_TEXTURE(run0_bmp),
        Assets_BITMAP_TEXTURE(run1_bmp)
    );

    Autorun_AddItem(
        "Extra1",
        Assets_BITMAP_TEXTURE(config0_bmp),
        Assets_BITMAP_TEXTURE(config1_bmp)
    );

    Autorun_AddItem(
        "Uninstall",
        Assets_BITMAP_TEXTURE(uninstall0_bmp),
        Assets_BITMAP_TEXTURE(uninstall1_bmp)
    );

    Autorun_AddItem(
        "Cancel",
        Assets_BITMAP_TEXTURE(cancel0_bmp),
        Assets_BITMAP_TEXTURE(cancel1_bmp)
    );
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
        SDL_Texture *cur_texture = item.texture;

        const SDL_FRect dst = {
            .x = (float)item.x,
            .y = (float)item.y,

            .w = (float)item.texture->w,
            .h = (float)item.texture->h,
        };

        switch (item.type)
        {
        case Autorun_ItemType_BUTTON:
            Utils_Vec2 mouse_pos;
            SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);

            cur_texture = Utils_PointInRect(mouse_pos, dst) ? item.button.hover_texture : item.texture;
            break;
        case Autorun_ItemType_VIDEO:
            break;
        }

        SDL_RenderTexture(Autorun_renderer, cur_texture, NULL, &dst);
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
