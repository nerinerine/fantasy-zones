#include "hotkey.hpp"

#include <fmt/core.h>

#include "../state.hpp"
#include "mouse.hpp"
#include "screen.hpp"

#include <Windows.h>

auto regHotkey() noexcept -> bool {
    const int keyId = 1;
    const auto _hwnd =
        SDL_GetPointerProperty(SDL_GetWindowProperties(getState().win),
                               SDL_PROP_WINDOW_WIN32_HWND_POINTER,
                               nullptr);
    const auto hwnd = static_cast<HWND>(_hwnd);

    if (!RegisterHotKey(hwnd, keyId, MOD_ALT, *"A")) {
        return false;
    }

    SDL_SetWindowsMessageHook(
        [](void *, MSG *msg) -> bool {
            static bool isWindowShow = true;
            if (msg->message == WM_HOTKEY && msg->wParam == keyId) {
                if (isWindowShow) {
                    SDL_HideWindow(getState().win);
                } else {
                    const auto pos = getMousePos();
                    const auto size = getScreenSize();
                    SDL_SetWindowPosition(getState().win,
                                          pos.x - getState().winWidth / 2,
                                          pos.y - getState().winHeight / 2);
                    SDL_ShowWindow(getState().win);
                }
                isWindowShow = !isWindowShow;
            }
            return true;
        },
        nullptr);

    return true;
}