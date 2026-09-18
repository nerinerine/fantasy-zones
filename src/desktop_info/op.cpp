#include "info.hpp"

auto operator/(const Pos &pos, const Rect &rect) -> RelPos {
    return { .x = static_cast<double>(pos.x) / rect.w,
             .y = static_cast<double>(pos.y) / rect.h };
}

auto operator==(const Rect &lhs, const Rect &rhs) -> bool {
    return lhs.w == rhs.w && lhs.h == rhs.h;
}