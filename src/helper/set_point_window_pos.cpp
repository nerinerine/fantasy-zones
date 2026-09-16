#include "set_point_window_pos.hpp"

#include <ranges>

#include <fmt/format.h>
#include "../desktop_info/info.hpp"

#include <windows.h>
#include <dwmapi.h>

using std::ranges::views::iota;

// 应当在sdl window隐藏后使用！！！
void setPointWindowPos(const GridItem *grid) {
    const auto pos = getMousePos();
    auto pointWindow = WindowFromPoint({ .x = pos.x, .y = pos.y });
    if (!pointWindow) [[unlikely]] {
        fmt::println(stderr, "error:no point window");
        return;
    }
    LONG_PTR style = GetWindowLongPtr(pointWindow, GWL_STYLE);

    for (const auto i : iota(0, 128)) {
        if ((style & WS_THICKFRAME)) {
            break;
        }
        pointWindow = GetParent(pointWindow);
        style = GetWindowLongPtr(pointWindow, GWL_STYLE);
    }

    if (!(style & WS_THICKFRAME)) [[unlikely]] {
        // 要是一直找不到有边框的窗口那就摆烂了...
        return;
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