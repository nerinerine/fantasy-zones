#include "grid.hpp"

#include <array>
#include <ranges>
#include <cstdint>
#include <mini/ini.h>
#include <fmt/base.h>
#include <SDL3/SDL.h>

#include "../desktop_info/info.hpp"
#include "gen_color.hpp"

constexpr uint8_t BORDER_ALPHA = UINT8_MAX;
double GridItem::shadowOffset{};

auto loadConfig(const std::string_view Path) -> GridList {
    GridList list{};

    mINI::INIFile configFile{ Path };
    mINI::INIStructure config{};
    if (!configFile.read(config)) {
        fmt::println(stderr, "can't load ini");
        return list;
    }

    for (auto const &[section, collection] : config) {
        if (!section.starts_with("grid")) {
            continue;
        }

        const auto x = std::stod(collection.get("x"));
        const auto y = std::stod(collection.get("y"));
        const auto w = std::stod(collection.get("w"));
        const auto h = std::stod(collection.get("h"));

        list.push_back({ .x = x, .y = y, .w = w, .h = h, .col = genColor() });
    }

    return list;
}

[[nodiscard]] auto GridItem::innerDisplayRect() const noexcept -> SDL_FRect {
    const auto size = getWindowSize(getScreenSize());

    const auto _innerW = static_cast<float>(size.w * w - BORDER * 2);
    const auto _innerH = static_cast<float>(size.h * h - BORDER * 2);

    const auto innerW = _innerW > 0 ? _innerW : 0;
    const auto innerH = _innerH > 0 ? _innerH : 0;

    return {
        .x = static_cast<float>(size.w * x + BORDER),
        .y = static_cast<float>(size.h * y + BORDER),
        .w = innerW,
        .h = innerH,
    };
}

[[nodiscard]] auto GridItem::outerDisplayRect() const noexcept -> SDL_FRect {
    const auto size = getWindowSize(getScreenSize());
    return {
        .x = static_cast<float>(size.w * x),
        .y = static_cast<float>(size.h * y),
        .w = static_cast<float>(size.w * w),
        .h = static_cast<float>(size.h * h),
    };
}

[[nodiscard]] auto GridItem::realRect() const noexcept -> SDL_Rect {
    const auto size = getScreenSize();
    return {
        .x = static_cast<int>(size.w * x),
        .y = static_cast<int>(size.h * y),
        .w = static_cast<int>(size.w * w),
        .h = static_cast<int>(size.h * h),
    };
}

void GridItem::drawBorder(SDL_Renderer *rdr) const noexcept {
    const auto &inner = innerDisplayRect();
    const auto &outer = outerDisplayRect();

    //  构造四个边框矩形，每条宽度为 BORDER，它们共同覆盖 outer 与 inner 之间的环形区域
    const std::array borderList{
        // outer左上至inner右下
        SDL_FRect{
            .x = outer.x, .y = outer.y, .w = BORDER, .h = inner.h + BORDER },
        // outer左下至inner右上
        SDL_FRect{ .x = outer.x,
                   .y = inner.y + inner.h,
                   .w = inner.w + BORDER,
                   .h = BORDER },
        //    outer右下至inner右上
        SDL_FRect{ .x = inner.x + inner.w,
                   .y = inner.y,
                   .w = BORDER,
                   .h = inner.h + BORDER },
        //    outer右上至inner左上
        SDL_FRect{
            .x = inner.x, .y = outer.y, .w = inner.w + BORDER, .h = BORDER },
    };
    SDL_SetRenderDrawColor(rdr, col.r, col.g, col.b, BORDER_ALPHA);
    SDL_RenderFillRects(rdr, borderList.data(), borderList.size());
}

void GridItem::drawInner(SDL_Renderer *rdr) const noexcept {
    constexpr uint8_t BG_ALPHA = UINT8_MAX / 4;
    const auto &inner = innerDisplayRect();

    SDL_SetRenderDrawColor(rdr, col.r, col.g, col.b, BG_ALPHA);
    SDL_RenderFillRect(rdr, &inner);
}

void GridItem::draw(SDL_Renderer *rdr) const noexcept {
    drawBorder(rdr);
    drawInner(rdr);
}

void GridItem::drawShadow(SDL_Renderer *rdr) const noexcept {
    using std::ranges::views::iota;
    SDL_SetRenderDrawColor(rdr, col.r, col.g, col.b, BORDER_ALPHA);

    const auto inner = innerDisplayRect();
    const int lineCount = floor((inner.w + inner.h) / SHADOW_SPACE);

    for (const auto i : iota(0, lineCount)) {
        // 这一段懒得写注释了，让我们看看D指导怎么说
        // 绘制动态对角线阴影线条，方向为从左上向右下移动（45° 斜线）。
        // 原理：以阴影间距 SHADOW_SPACE 为步长，分别遍历整个内矩形区域的上右边和左下边。
        // 这样所有线条均平行且均匀分布，形成斜线纹理。
        // 同时，shadowOffset 随时间递增并取模，使线条整体平移，产生流动动画效果。

        // 以及，shadowOffset仅在SHADOW_SPACE范围内循环，
        const auto pos = static_cast<float>(i * SHADOW_SPACE + shadowOffset);
        const double x1 = pos > inner.w ? inner.x + inner.w : inner.x + pos;
        const double y1 = pos > inner.w ? pos - inner.w + inner.y : inner.y;

        const double x2 = pos > inner.h ? pos - inner.h + inner.x : inner.x;
        const double y2 = pos > inner.h ? inner.y + inner.h : inner.y + pos;

        SDL_RenderLine(rdr,
                       static_cast<float>(x1),
                       static_cast<float>(y1),
                       static_cast<float>(x2),
                       static_cast<float>(y2));
    }

    shadowOffset = std::fmod(shadowOffset + 1, SHADOW_SPACE);
}
