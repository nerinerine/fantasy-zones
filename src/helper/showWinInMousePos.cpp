#include "showWinInMousePos.hpp"
#include "mouse.hpp"
#include "screen.hpp"

void showWinInMousePos(SDL_Window *win) noexcept {
    const auto pos = getMousePos();
    const auto size = getScreenSize();
    int winWidth{};
    int winHeight{};
    SDL_GetWindowSize(win, &winWidth, &winHeight);
    const double x = static_cast<double>(pos.x) / size.width;
    const double y = static_cast<double>(pos.y) / size.height;
    SDL_SetWindowPosition(win,
                          static_cast<int>(pos.x - winWidth * x),
                          static_cast<int>(pos.y - winHeight * y));
    SDL_ShowWindow(win);
}