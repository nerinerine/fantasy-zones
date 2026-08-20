#include "set_point_window_pos.hpp"

#include "../desktop_info/info.hpp"

#include <windows.h>

void setPointWindowPos(const GridItem *grid) noexcept {
    const auto pos = getMousePos();
    auto pointWindow = WindowFromPoint({ .x = pos.x, .y = pos.y });
    LONG_PTR style = GetWindowLongPtr(pointWindow, GWL_STYLE);
    while (!(style & WS_THICKFRAME)) {
        pointWindow = GetParent(pointWindow);
        style = GetWindowLongPtr(pointWindow, GWL_STYLE);
    }
    const auto toRect = grid->realRect();
    SetWindowPos(
        pointWindow, nullptr, toRect.x, toRect.y, toRect.w, toRect.h, 0);
}