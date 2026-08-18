#include "info.hpp"
#include <windows.h>

[[nodiscard]] auto getMousePos() -> Pos {
    POINT pt;
    GetCursorPos(&pt);
    return { .x = pt.x, .y = pt.y };
}