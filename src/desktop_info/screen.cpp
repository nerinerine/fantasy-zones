#include <Windows.h>
#include "./info.hpp"
#include "../define.hpp"

auto getScreenSize() -> Rect {
    static const auto w{ GetSystemMetrics(SM_CXSCREEN) };
    static const auto h{ GetSystemMetrics(SM_CYSCREEN) };
    return { .width = w, .height = h };
}

auto getWindowSize(Rect size) -> Rect {
    return { .width = static_cast<int>(size.width * WINDOW_WIDTH_SCALE),
             .height = static_cast<int>(size.height * WINDOW_HEIGHT_SCALE) };
}