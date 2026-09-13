#pragma once
#include <SDL3/SDL.h>

struct GuiContext {
    SDL_Renderer *rdr;
    SDL_Window *win;
};

void runGui(const GuiContext ctx);