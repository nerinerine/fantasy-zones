#include <Windows.h>
#include "./screen.hpp"

auto getScreenSize() -> rect {
    static const auto w{ GetSystemMetrics(SM_CXSCREEN) };
    static const auto h{ GetSystemMetrics(SM_CYSCREEN) };
    return { .width = w, .height = h };
}