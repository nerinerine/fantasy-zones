#include "hotkey.hpp"

#include <fmt/core.h>

#include "mouse.hpp"
#include "screen.hpp"

#include <Windows.h>

auto regHotkey(SDL_Window *window) noexcept -> bool {
    const int keyId = 1;
    const auto _hwnd =
        SDL_GetPointerProperty(SDL_GetWindowProperties(window),
                               SDL_PROP_WINDOW_WIN32_HWND_POINTER,
                               nullptr);
    const auto hwnd = reinterpret_cast<HWND>(_hwnd);

    if (!RegisterHotKey(hwnd, keyId, 0, VK_MENU)) {
        return false;
    }

    SDL_SetWindowsMessageHook(
        [](void *_window, MSG *msg) -> bool {
            static bool isWindowShow = true;
            const auto window = reinterpret_cast<SDL_Window *>(_window);
            if (msg->message == WM_HOTKEY && msg->wParam == keyId) {
                if (isWindowShow) {
                    SDL_HideWindow(window);
                } else {
                    const auto pos = getMousePos();
                    const auto size = getScreenSize();
                    SDL_SetWindowPosition(window,
                                          pos.x - size.width / 8,
                                          pos.y - size.height / 8);
                    SDL_ShowWindow(window);
                }
                isWindowShow = !isWindowShow;
            }
            return true;
        },
        window);

    return true;
}