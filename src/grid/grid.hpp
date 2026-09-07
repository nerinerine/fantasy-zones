#pragma once
#include <vector>
#include <filesystem>
#include <SDL3/SDL.h>

struct DRect {
    static constexpr int BORDER = 5;
    static constexpr int SHADOW_SPACE = 10;
    static double shadowOffset;
    double x{};
    double y{};
    double w{};
    double h{};
    // 计算内部矩形
    [[nodiscard]] auto innerDisplayRect() const noexcept -> SDL_FRect;
    // 计算外部矩形
    [[nodiscard]] auto outerDisplayRect() const noexcept -> SDL_FRect;
    // 计算窗口实际将要移到的矩形
    [[nodiscard]] auto realRect() const noexcept -> SDL_Rect;
    void drawBorder(const SDL_Color col, SDL_Renderer *rdr) const;
    void drawInner(const SDL_Color col, SDL_Renderer *rdr) const;
    void drawShadow(const SDL_Color col, SDL_Renderer *rdr) const;
};

struct GridItem {
    bool visible{};
    DRect hitRect{};
    DRect targetRect{};

    SDL_Color col{};
};

using GridList = std::vector<GridItem>;

auto loadConfig(const std::filesystem::path &path) -> GridList;