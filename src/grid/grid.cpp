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

auto loadConfig(const std::string &Path) -> GridList {
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

    const std::array borderList{
        SDL_FRect{
            .x = outer.x, .y = outer.y, .w = BORDER, .h = inner.h + BORDER },
        SDL_FRect{ .x = outer.x,
                   .y = inner.y + inner.h,
                   .w = inner.w + BORDER,
                   .h = BORDER },
        SDL_FRect{ .x = inner.x + inner.w,
                   .y = inner.y,
                   .w = BORDER,
                   .h = inner.h + BORDER },
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

    auto inner = innerDisplayRect();

    for (auto i = shadowOffset; i < inner.w + inner.h; i += 10) {
        const double x1 = i > inner.w ? inner.x + inner.w : inner.x + i;
        const double y1 = i > inner.w ? i - inner.w + inner.y : inner.y;
        const double x2 = i > inner.h ? i - inner.h + inner.x : inner.x;
        const double y2 = i > inner.h ? inner.y + inner.h : inner.y + i;

        SDL_RenderLine(rdr,
                       static_cast<float>(x1),
                       static_cast<float>(y1),
                       static_cast<float>(x2),
                       static_cast<float>(y2));
    }

    shadowOffset = std::fmod(shadowOffset + 1, 10);
}
