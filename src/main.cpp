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
#include "res/media.h"
#include "utils.h"

extern "C"
{
#include <iniparser.h>
}

#include <smacker.h>

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

        struct
        {
            smk smacker;
            SDL_Palette *palette;

            double timer;
            double fps;

            int loops;
        } video;
    };
};

std::map<std::string, Autorun_Item> Autorun_items;

SDL_Window *Autorun_window;
SDL_Renderer *Autorun_renderer;

dictionary *Autorun_ini;

SDL_Texture *Autorun_backgroundTexture;

void Autorun_WriteVideoFrame(Autorun_Item *item)
{
    const unsigned char *palette = smk_get_palette(item->video.smacker);

    for (int i = 0; i < 256; i++)
    {
        SDL_Color color;
        color.r = palette[(i * 3) + 0];
        color.g = palette[(i * 3) + 1];
        color.b = palette[(i * 3) + 2];
        color.a = 255;

        SDL_SetPaletteColors(item->video.palette, &color, i, 1);
    }

    SDL_Surface *targ_surface = NULL;

    const unsigned char *frame = smk_get_video(item->video.smacker);
    SDL_Surface *tmp_surface = SDL_CreateSurfaceFrom(
        item->texture->w, item->texture->h,
        SDL_PIXELFORMAT_INDEX8,
        (void *)frame,
        item->texture->w
    );

    SDL_SetSurfacePalette(tmp_surface, item->video.palette);
    SDL_LockTextureToSurface(item->texture, NULL, &targ_surface);

    SDL_BlitSurface(tmp_surface, NULL, targ_surface, NULL);

    SDL_UnlockTexture(item->texture);
    SDL_DestroySurface(tmp_surface);
}

void Autorun_AddVideo(std::string name, void *data, size_t length)
{
    Autorun_Item item;
    item.type = Autorun_ItemType_VIDEO;

    item.x = iniparser_getint(Autorun_ini, (name + ":XPos").c_str(), 0);
    item.y = iniparser_getint(Autorun_ini, (name + ":YPos").c_str(), 0);

    item.video.loops = iniparser_getint(Autorun_ini, (name + ":Loops").c_str(), 0);
    item.video.smacker = smk_open_memory((const unsigned char *)data, length);

    double usf;
    unsigned long w, h;
    smk_info_video(item.video.smacker, &w, &h, NULL);
    smk_info_all(item.video.smacker, NULL, NULL, NULL, &usf);

    item.video.timer = 0.0;

    item.video.fps = 1e6 / usf;
    item.texture = SDL_CreateTexture(Autorun_renderer, SDL_PIXELFORMAT_RGBX32, SDL_TEXTUREACCESS_STREAMING, w, h);
    if (!item.texture)
    {
        SDL_Log("Failed to create smacker texture: %s", SDL_GetError());
        smk_close(item.video.smacker);
        return;
    }

    item.video.palette = SDL_CreatePalette(256);

    smk_enable_video(item.video.smacker, 1);
    smk_first(item.video.smacker);

    Autorun_WriteVideoFrame(&item);

    Autorun_items[name] = item;
}

void Autorun_AddItem(std::string name, SDL_Texture *texture, SDL_Texture *hover_texture)
{
    Autorun_Item item;
    item.type = Autorun_ItemType_BUTTON;

    item.x = iniparser_getint(
        Autorun_ini,
        (name + ":PostXPos").c_str(),

        iniparser_getint(Autorun_ini, (name + ":XPos").c_str(), 0)
    );

    item.y = iniparser_getint(
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
    Autorun_AddVideo(
        "Animate",
        installs_smk,
        installs_smk_len
    );

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
    SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);

    SDL_Surface *icon = Assets_LOAD_BITMAP(icon_bmp);

    Autorun_ini = iniparser_load("autorun.ini");
    if (!Autorun_ini)
    {
        SDL_Log("Failed to load autorun.ini");
        return SDL_APP_FAILURE;
    }

    const char *title = iniparser_getstring(Autorun_ini, "Instance:ProgramName", "");

    SDL_CreateWindowAndRenderer(title, 640, 480, SDL_WINDOW_BORDERLESS, &Autorun_window, &Autorun_renderer);

    SDL_SetRenderVSync(Autorun_renderer, SDL_RENDERER_VSYNC_ADAPTIVE);

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
        switch (item.type)
        {
        case Autorun_ItemType_BUTTON:
            SDL_DestroyTexture(item.button.hover_texture);
            break;
        case Autorun_ItemType_VIDEO:
            SDL_DestroyPalette(item.video.palette);
            smk_close(item.video.smacker);
            break;
        }

        SDL_DestroyTexture(item.texture);
    }

    SDL_DestroyRenderer(Autorun_renderer);
    SDL_DestroyWindow(Autorun_window);
}

double Autorun_lastTime = 0;

SDL_AppResult SDL_AppIterate(void *appstate)
{
    double current_time = SDL_GetTicksNS() / 1e9;
    double delta = current_time - Autorun_lastTime;
    Autorun_lastTime = current_time;

    // SDL_Log("DT: %lf", delta);

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

            .w = (float)cur_texture->w,
            .h = (float)cur_texture->h,
        };

        switch (item.type)
        {
        case Autorun_ItemType_BUTTON:
            Utils_Vec2 mouse_pos;
            SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);

            cur_texture = Utils_PointInRect(mouse_pos, dst) ? item.button.hover_texture : item.texture;
            break;
        case Autorun_ItemType_VIDEO:
            unsigned long frame, frame_count;
            smk_info_all(item.video.smacker, &frame, &frame_count, 0, 0);
            // SDL_Log("%ld, %ld", frame, frame_count);

            if (frame >= frame_count - 1 && item.video.loops > 0)
            {
                smk_first(item.video.smacker);
                item.video.loops--;
            }

            item.video.timer += delta;
            // SDL_Log("TIMER: %lf", item.video.timer);
            if (item.video.timer > 1.0 / item.video.fps)
            {
                item.video.timer = 0.0;
                Autorun_WriteVideoFrame(&item);
                smk_next(item.video.smacker);
            }

            Autorun_items[iter->first] = item;

            break;
        }

        SDL_RenderTexture(Autorun_renderer, cur_texture, NULL, &dst);
    }

    SDL_RenderPresent(Autorun_renderer);

    // SDL_Delay(40);
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
