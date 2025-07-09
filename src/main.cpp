// bits of mpeg SDL stuff by PhobosLabs (https://github.com/phoboslab/pl_mpeg/blob/master/pl_mpeg_player_sdl.c)

#include <stdio.h>
#include "pl_mpeg.h"

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "res/bitmaps.h"

bool running = true;

struct Autorun_Streamer {
    SDL_Texture *texture;
};

void app_on_video(plm_t *mpeg, plm_frame_t *frame, void *user) {
    Autorun_Streamer *self = (Autorun_Streamer *)user;

    SDL_UpdateYUVTexture(self->texture, NULL, frame->y.data, frame->y.width, frame->cb.data, frame->cb.width, frame->cr.data,  frame->cr.width);
}

void app_on_audio(plm_t *mpeg, plm_samples_t *samples, void *user) {
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    if (!running)
        return SDL_APP_SUCCESS;

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    if (event->type == SDL_EVENT_QUIT) {
        running = false;
    }

    return SDL_APP_CONTINUE;
}
