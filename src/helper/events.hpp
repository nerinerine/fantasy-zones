#pragma once
#include <SDL3/SDL.h>
struct Events {
    SDL_Event displayEvent;
    SDL_Event submitEvent;
    SDL_Event cancelEvent;
};