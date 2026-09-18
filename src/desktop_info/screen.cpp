#include "info.hpp"

#include <fmt/format.h>

#include <Windows.h>
#include <WinUser.h>

constexpr auto WINDOW_WIDTH_SCALE = 0.25;
constexpr auto WINDOW_HEIGHT_SCALE = 0.25;

auto getArea() -> RECT {
    static Pos mousePos{};
    static MONITORINFO minfo{};
    RECT workArea{};

    mousePos = getMousePos();
    HMONITOR monitor = MonitorFromPoint({ .x = mousePos.x, .y = mousePos.y },
                                        MONITOR_DEFAULTTONEAREST);
    // hyw
    minfo = { .cbSize = sizeof(MONITORINFO) };
    if (GetMonitorInfo(monitor, &minfo)) {
        workArea = minfo.rcWork;
    } else {
        fmt::print("can't get monitor info");
    }
    return workArea;

    const int workWidth = workArea.right - workArea.left;
    const int workHeight = workArea.bottom - workArea.top;
}

auto getScreenSize() -> Rect {
    // static const auto w{ GetSystemMetrics(SM_CXSCREEN) };
    // static const auto h{ GetSystemMetrics(SM_CYSCREEN) };
    static Pos mousePos{};
    static MONITORINFO minfo{};
    auto workArea = getArea();

    const int workWidth = workArea.right - workArea.left;
    const int workHeight = workArea.bottom - workArea.top;

    return { .w = workWidth, .h = workHeight };
}

auto getScreenStartPoint() -> Pos {
    auto workArea = getArea();
    return { .x = workArea.left, .y = workArea.top };
}

auto getWindowSize(Rect size) -> Rect {
    return { .w = static_cast<int>(size.w * WINDOW_WIDTH_SCALE),
             .h = static_cast<int>(size.h * WINDOW_HEIGHT_SCALE) };
}