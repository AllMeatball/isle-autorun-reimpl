// Copyright (C) 2025 AllMeatball
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _UTILS_H
#define _UTILS_H

#include <string.h>
#include <string>
#include <thread>
#include <vector>

#include <SDL3/SDL.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct
{
    float x, y;
} Utils_Vec2;

inline bool Utils_PointInRect(Utils_Vec2 point, SDL_FRect rect)
{
    return (point.x > rect.x && point.x < rect.x + rect.w) &&
           (point.y > rect.y && point.y < rect.y + rect.h);
}

inline int Utils_RunCommand(std::string cmd, std::string arg_string)
{
    pid_t pid = fork();
    if (pid == 0) {
        std::vector<char *> args;
        char *arg_cstr = strdup(arg_string.c_str());

        char *save_ptr = NULL;

        args.push_back((char*)cmd.c_str());

        char *token = strtok_r(arg_cstr, " ", &save_ptr);
        args.push_back(token);

        while (token != NULL)
        {
            token = strtok_r(NULL, " ", &save_ptr);
            args.push_back(token);
        }
        // args.push_back((char*)"hello");
        args.push_back(NULL);

        int ret = execvp(cmd.c_str(), args.data());

        // SDL_Log("Child fork");
        if (ret == -1)
        {
            exit(1);
        }
    }
    // free(arg_cstr);

    return 0;
}

#endif
