#include "set_point_window_pos.hpp"

#include "../desktop_info/info.hpp"

#include <windows.h>
#include <dwmapi.h>

void setPointWindowPos(const GridItem *grid) noexcept {
    const auto pos = getMousePos();
    auto pointWindow = WindowFromPoint({ .x = pos.x, .y = pos.y });
    LONG_PTR style = GetWindowLongPtr(pointWindow, GWL_STYLE);
    while (!(style & WS_THICKFRAME)) {
        pointWindow = GetParent(pointWindow);
        style = GetWindowLongPtr(pointWindow, GWL_STYLE);
    }

    RECT windowRect{}; //窗口的大小
    RECT shadowRect{}; //算上阴影之后窗口的真实大小
    GetWindowRect(pointWindow, &windowRect);
    DwmGetWindowAttribute(pointWindow,
                          DWMWA_EXTENDED_FRAME_BOUNDS,
                          &shadowRect,
                          sizeof(RECT));
    int offsetX = shadowRect.left - windowRect.left;
    int offsetY = shadowRect.top - windowRect.top;

    const auto toRect = grid->targetRect.realRect();
    SetWindowPos(pointWindow,
                 nullptr,
                 toRect.x - offsetX,
                 toRect.y - offsetY,
                 toRect.w + 2 * offsetX,
                 toRect.h + 2 * offsetY,
                 0);
}