#ifndef _UTILS_H
#define _UTILS_H

#include <SDL3/SDL.h>

typedef struct
{
    float x, y;
} Utils_Vec2;

inline bool Utils_PointInRect(Utils_Vec2 point, SDL_FRect rect)
{
    return (point.x > rect.x && point.x < rect.x + rect.w) &&
           (point.y > rect.y && point.y < rect.y + rect.h);
}

#endif
