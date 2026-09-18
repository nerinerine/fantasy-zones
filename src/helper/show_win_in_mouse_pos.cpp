#include "show_win_in_mouse_pos.hpp"
#include "../desktop_info/info.hpp"

auto showWinInMousePos(SDL_Window *win) -> SDL_Point {
    const auto size = getScreenSize();
    const auto screenPos = getScreenStartPoint();
    const auto mousePos = getMousePos();
    // 鼠标在当前屏幕上的位置
    const Pos pos = { .x = mousePos.x - screenPos.x,
                      .y = mousePos.y - screenPos.y };
    const auto relPos = pos / size;
    static int winWidth{};
    static int winHeight{};
    SDL_GetWindowSize(win, &winWidth, &winHeight);
    const int winX = mousePos.x - static_cast<int>(winWidth * relPos.x);
    const int winY = mousePos.y - static_cast<int>(winHeight * relPos.y);
    SDL_SetWindowPosition(win, winX, winY);
    SDL_ShowWindow(win);
    return { .x = winX, .y = winY };
}