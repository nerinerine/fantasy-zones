#include "showWinInMousePos.hpp"
#include "mouse.hpp"

void showWinInMousePos(SDL_Window *win) noexcept {
    const auto pos = getMousePos();
    int winWidth{};
    int winHeight{};
    SDL_GetWindowSize(win, &winWidth, &winHeight);
    SDL_SetWindowPosition(win, pos.x - winWidth / 2, pos.y - winHeight / 2);
    SDL_ShowWindow(win);
}