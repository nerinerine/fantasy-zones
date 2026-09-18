#include "info.hpp"
#include <windows.h>

[[nodiscard]] auto getMousePos() -> Pos {
    static POINT pt;
    GetCursorPos(&pt);
    return { .x = pt.x, .y = pt.y };
}