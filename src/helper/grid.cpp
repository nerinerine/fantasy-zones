#include "grid.hpp"
#include <mini/ini.h>
#include <fmt/base.h>

#include "../desktop_info/info.hpp"

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

        list.push_back({ .x = x, .y = y, .w = w, .h = h });
    }

    return list;
}

auto GridItem::displayRect() noexcept -> SDL_Rect {
    const auto size = getWindowSize(getScreenSize());
    return {
        .x = static_cast<int>(size.w * x),
        .y = static_cast<int>(size.h * y),
        .w = static_cast<int>(size.w * w),
        .h = static_cast<int>(size.h * h),
    };
}
auto GridItem::realRect() noexcept -> SDL_Rect {
    const auto size = getScreenSize();
    return {
        .x = static_cast<int>(size.w * x),
        .y = static_cast<int>(size.h * y),
        .w = static_cast<int>(size.w * w),
        .h = static_cast<int>(size.h * h),
    };
}