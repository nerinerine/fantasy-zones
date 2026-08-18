#include <Windows.h>
#include "info.hpp"
#include "../define.hpp"

auto getScreenSize() -> Rect {
    static const auto w{ GetSystemMetrics(SM_CXSCREEN) };
    static const auto h{ GetSystemMetrics(SM_CYSCREEN) };
    return { .w = w, .h = h };
}

auto getWindowSize(Rect size) -> Rect {
    return { .w = static_cast<int>(size.w * WINDOW_WIDTH_SCALE),
             .h = static_cast<int>(size.h * WINDOW_HEIGHT_SCALE) };
}