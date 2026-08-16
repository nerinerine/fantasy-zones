#include "showWinInMousePos.hpp"
#include "../desktop_info/info.hpp"

void showWinInMousePos(SDL_Window *win) noexcept {
    const auto pos = getMousePos();
    const auto size = getScreenSize();
    const auto relPos = pos / size;
    int winWidth{};
    int winHeight{};
    SDL_GetWindowSize(win, &winWidth, &winHeight);
    SDL_SetWindowPosition(win,
                          static_cast<int>(pos.x - winWidth * relPos.x),
                          static_cast<int>(pos.y - winHeight * relPos.y));
    SDL_ShowWindow(win);
}