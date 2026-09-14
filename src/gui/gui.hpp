#pragma once
#include <SDL3/SDL.h>
#include "../grid/grid.hpp"

struct GuiContext {
    SDL_Renderer *rdr;
    SDL_Window *win;
    const GridList* list;
};

void runGui(const GuiContext ctx);