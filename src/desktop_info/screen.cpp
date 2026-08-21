#include <Windows.h>
#include "info.hpp"
#include "../define.hpp"

auto getScreenSize() -> Rect {
    // static const auto w{ GetSystemMetrics(SM_CXSCREEN) };
    // static const auto h{ GetSystemMetrics(SM_CYSCREEN) };

    HMONITOR monitor = MonitorFromWindow(nullptr, MONITOR_DEFAULTTOPRIMARY);
    // hyw
    MONITORINFO minfo = { .cbSize = sizeof(MONITORINFO) };
    RECT workArea;
    if (GetMonitorInfo(monitor, &minfo)) {
        workArea = minfo.rcWork;
    }

    const int workWidth = workArea.right - workArea.left;
    const int workHeight = workArea.bottom - workArea.top;

    return { .w = workWidth, .h = workHeight };
}

auto getWindowSize(Rect size) -> Rect {
    return { .w = static_cast<int>(size.w * WINDOW_WIDTH_SCALE),
             .h = static_cast<int>(size.h * WINDOW_HEIGHT_SCALE) };
}