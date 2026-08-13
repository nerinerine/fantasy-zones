#pragma once
#include <SDL3/SDL.h>
struct State {
    int winWidth;
    int winHeight;
    SDL_Window *win;
};

auto setState() noexcept -> State &;
auto getState() noexcept -> const State&;