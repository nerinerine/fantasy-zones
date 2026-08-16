#include "info.hpp"

auto operator/(Pos pos, Rect rect) -> RelPos {
    return { .x = static_cast<double>(pos.x) / rect.width,
             .y = static_cast<double>(pos.y) / rect.height };
}