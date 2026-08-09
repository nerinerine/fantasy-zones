#pragma once
#include <SDL3/SDL.h>
struct State {
    int winWidth;
    int winHeight;
    SDL_Window *win;
};

auto getState() noexcept -> State &;