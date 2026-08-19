#pragma once
#include <vector>
#include <string_view>
#include <SDL3/SDL.h>

struct GridItem {
    static constexpr int BORDER = 5;
    static constexpr int SHADOW_SPACE = 10;
    static double shadowOffset;
    double x{};
    double y{};
    double w{};
    double h{};
    SDL_Color col{};
    // 计算内部矩形
    [[nodiscard]] auto innerDisplayRect() const noexcept -> SDL_FRect;
    // 计算外部矩形
    [[nodiscard]] auto outerDisplayRect() const noexcept -> SDL_FRect;
    // 计算窗口实际将要移到的矩形
    [[nodiscard]] auto realRect() const noexcept -> SDL_Rect;
    void drawBorder(SDL_Renderer *rdr) const noexcept;
    void drawInner(SDL_Renderer *rdr) const noexcept;
    void draw(SDL_Renderer *rdr) const noexcept;
    void drawShadow(SDL_Renderer *rdr) const noexcept;
};

using GridList = std::vector<GridItem>;

auto loadConfig(const std::string_view Path) -> GridList;