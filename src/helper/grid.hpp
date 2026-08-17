#pragma once
#include <vector>
#include <string>
#include <SDL3/SDL.h>

struct GridItem {
    double x{};
    double y{};
    double w{};
    double h{};
    auto displayRect() noexcept -> SDL_Rect;
    auto realRect() noexcept -> SDL_Rect;
};

using GridList = std::vector<GridItem>;

auto loadConfig(std::string &Path) -> GridList;