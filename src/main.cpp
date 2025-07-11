// Copyright (C) 2025 AllMeatball
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <assert.h>
#include <stdio.h>

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <map>
#include <string>
#include <vector>

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

typedef struct Autorun_Item_STRUCT Autorun_Item;

typedef void (*Autorun_ButtonAction)(Autorun_Item);

struct Autorun_Item_STRUCT
{
    enum Autorun_ItemType type;
    std::string name;

    int x, y;
    SDL_Texture *texture;

    union
    {
        struct
        {
            bool is_hovering;
            Autorun_ButtonAction action;

            SDL_Texture *hover_texture;
        } button;

        struct
        {
            smk smacker;
            SDL_Palette *palette;

            double timer;
            double fps;

            int loops;
            bool paused;
        } video;
    };
};

std::map<std::string, Autorun_Item> Autorun_items;

const char *Autorun_title = "";
SDL_Window *Autorun_window;
SDL_Renderer *Autorun_renderer;
SDL_AudioStream *Autorun_audioStream;
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
    item.name = name;

    item.x = iniparser_getint(Autorun_ini, (name + ":XPos").c_str(), 0);
    item.y = iniparser_getint(Autorun_ini, (name + ":YPos").c_str(), 0);

    item.video.loops = iniparser_getint(Autorun_ini, (name + ":Loops").c_str(), 0);
    item.video.smacker = smk_open_memory((const unsigned char *)data, length);

    double usf;
    unsigned long w, h;
    smk_info_video(item.video.smacker, &w, &h, NULL);
    smk_info_all(item.video.smacker, NULL, NULL, NULL, &usf);

    item.video.timer = 0.0;
    item.video.paused = false;

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
    smk_enable_audio(item.video.smacker, 0, 1);
    smk_first(item.video.smacker);

    Autorun_WriteVideoFrame(&item);

    Autorun_items[name] = item;
}

void Autorun_AddButton(std::string name, SDL_Texture *texture, SDL_Texture *hover_texture, Autorun_ButtonAction action)
{
    Autorun_Item item;
    item.type = Autorun_ItemType_BUTTON;
    item.name = name;

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
    item.button.is_hovering = false;
    item.button.action = action;
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

    Autorun_AddButton(
        "Run",
        Assets_BITMAP_TEXTURE(run0_bmp),
        Assets_BITMAP_TEXTURE(run1_bmp),
        [](Autorun_Item self)
        {
            SDL_ShowSimpleMessageBox(
                SDL_MESSAGEBOX_INFORMATION,
                Autorun_title,
                "TODO: make this open LEGO Island",
                Autorun_window
            );
        }
    );

    Autorun_AddButton(
        "Extra1",
        Assets_BITMAP_TEXTURE(config0_bmp),
        Assets_BITMAP_TEXTURE(config1_bmp),
        [](Autorun_Item self)
        {
            SDL_ShowSimpleMessageBox(
                SDL_MESSAGEBOX_INFORMATION,
                Autorun_title,
                "TODO: make this open the config app",
                Autorun_window
            );
        }
    );

    Autorun_AddButton(
        "Uninstall",
        Assets_BITMAP_TEXTURE(uninstall0_bmp),
        Assets_BITMAP_TEXTURE(uninstall1_bmp),
        [](Autorun_Item self)
        {
            SDL_ShowSimpleMessageBox(
                SDL_MESSAGEBOX_INFORMATION,
                Autorun_title,
                "You can delete the files manually.\nNo uninstaller is program needed.",
                Autorun_window
            );
        }
    );

    Autorun_AddButton(
        "Cancel",
        Assets_BITMAP_TEXTURE(cancel0_bmp),
        Assets_BITMAP_TEXTURE(cancel1_bmp),
        [](Autorun_Item self)
        {
            running = false;
        }
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

    Autorun_title = iniparser_getstring(Autorun_ini, "Instance:ProgramName", "");

    SDL_CreateWindowAndRenderer(Autorun_title, 640, 480, SDL_WINDOW_BORDERLESS, &Autorun_window, &Autorun_renderer);

    SDL_SetRenderVSync(Autorun_renderer, SDL_RENDERER_VSYNC_ADAPTIVE);

    SDL_SetWindowIcon(Autorun_window, icon);
    SDL_DestroySurface(icon);

    Autorun_LoadItems();
    Autorun_backgroundTexture = Assets_BITMAP_TEXTURE(autorun_bmp);

    SDL_AudioSpec spec;

    smk smacker = Autorun_items["Animate"].video.smacker;
    unsigned char channels, bit_depth;
    unsigned long freq;

    smk_info_audio(smacker, NULL, &channels, &bit_depth, &freq);

    // SDL_Log("%d")
    spec.channels = channels;
    spec.format = SDL_AUDIO_S16LE;
    spec.freq = freq;

    Autorun_audioStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
    SDL_ResumeAudioStreamDevice(Autorun_audioStream);

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

    SDL_DestroyAudioStream(Autorun_audioStream);
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
            bool hovering;

            SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);

            hovering = Utils_PointInRect(mouse_pos, dst);
            item.button.is_hovering = hovering;

            Autorun_items[iter->first] = item;

            cur_texture = hovering ? item.button.hover_texture : item.texture;
            break;
        case Autorun_ItemType_VIDEO:
            unsigned long frame, frame_count;

            if (item.video.paused)
                break;

            smk_info_all(item.video.smacker, &frame, &frame_count, 0, 0);
            // SDL_Log("%ld, %ld", frame, frame_count);

            if (frame >= frame_count - 1 && item.video.loops > 0)
            {
                smk_first(item.video.smacker);
                item.video.loops--;
            }

            if (item.video.loops < 1) {
                item.video.paused = true;
                break;
            }

            item.video.timer += delta;
            // SDL_Log("TIMER: %lf", item.video.timer);
            if (item.video.timer > 1.0 / item.video.fps)
            {
                item.video.timer = 0.0;
                Autorun_WriteVideoFrame(&item);

                const void *audio_buf = smk_get_audio(item.video.smacker, 0);
                unsigned long audio_buf_len = smk_get_audio_size(item.video.smacker, 0);

                SDL_PutAudioStreamData(Autorun_audioStream, audio_buf, audio_buf_len);
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
        running = false;

    if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN)
    {
        if (event->button.button != SDL_BUTTON_LEFT)
            return SDL_APP_CONTINUE;

        std::map<std::string, Autorun_Item>::iterator iter;
        for (iter = Autorun_items.begin(); iter != Autorun_items.end(); iter++)
        {
            Autorun_Item item = iter->second;

            if (item.type != Autorun_ItemType_BUTTON)
                continue;

            if (!item.button.is_hovering)
                continue;

            item.button.action(item);
            Autorun_items["Animate"].video.paused = true;
            break;
        }
    }

    return SDL_APP_CONTINUE;
}
