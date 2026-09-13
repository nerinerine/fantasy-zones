#pragma once
#include <SDL3/SDL.h>
#include "../helper/event.hpp"

struct GuiContext {
    EventQueue *queue;
    SDL_Window *window;
};

auto runGui(const GuiContext ctx) -> int;