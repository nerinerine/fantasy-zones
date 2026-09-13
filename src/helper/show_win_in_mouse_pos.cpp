#include "show_win_in_mouse_pos.hpp"
#include "../desktop_info/info.hpp"

auto showWinInMousePos(SDL_Window *win) noexcept -> SDL_Point {
    const auto pos = getMousePos();
    const auto size = getScreenSize();
    const auto relPos = pos / size;
    int winWidth{};
    int winHeight{};
    SDL_GetWindowSize(win, &winWidth, &winHeight);
    const int winX = static_cast<int>(pos.x - winWidth * relPos.x);
    const int winY = static_cast<int>(pos.y - winHeight * relPos.y);
    SDL_SetWindowPosition(win, winX, winY);
    SDL_ShowWindow(win);
    return { .x = winX, .y = winY };
}