#include "set_point_window_pos.hpp"

#include "../desktop_info/info.hpp"
#include <windows.h>

void setPointWindowPos(GridItem &grid) noexcept {
    const auto pos = getMousePos();
    const auto pointWindow = WindowFromPoint({ .x = pos.x, .y = pos.y });
    const auto toRect = grid.realRect();
    SetWindowPos(
        pointWindow, nullptr, toRect.x, toRect.y, toRect.w, toRect.h, 0);
}