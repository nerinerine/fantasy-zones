#pragma once
#include <vector>
#include <string>
#include <SDL3/SDL.h>

struct GridItem {
    static constexpr int BORDER = 5;
    double x{};
    double y{};
    double w{};
    double h{};
    SDL_Color col{};
    [[nodiscard]] auto innerDisplayRect() const noexcept -> SDL_FRect;
    [[nodiscard]] auto outerDisplayRect() const noexcept -> SDL_FRect;
    [[nodiscard]] auto realRect() const noexcept -> SDL_Rect;
    void drawBorder(SDL_Renderer *rdr) const noexcept;
    void drawInner(SDL_Renderer *rdr) const noexcept;
    void draw(SDL_Renderer *rdr) const noexcept;
};

using GridList = std::vector<GridItem>;

auto loadConfig(const std::string &Path) -> GridList;