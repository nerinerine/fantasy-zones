#include <thread>
#include <clocale>

#include <fmt/base.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_tray.h>

#include "gui/gui.hpp"
#include "llhook/hook.hpp"
#include "desktop_info/info.hpp"
#include "helper/make_unique_with.hpp"

auto main(int argc, char **argv) -> int {
    std::setlocale(LC_ALL,
                   ""); //使用用户环境默认locale以使fmt可以正常打印宽字符串

    // init sdl
    if (!SDL_Init(SDL_INIT_VIDEO)) [[unlikely]] {
        fmt::println(stderr, "sdl init error:{}", SDL_GetError());
        return -1;
    }

    const auto SDLModelHandle = MakeUniqueWith<SDL_Quit>();

    // create window
    const auto windowSize = getWindowSize(getScreenSize());

    const auto win = MakeUniqueWith<SDL_DestroyWindow>(
        SDL_CreateWindow("Fantacy Zone",
                         windowSize.w,
                         windowSize.h,
                         SDL_WINDOW_BORDERLESS | SDL_WINDOW_HIDDEN));

    if (!win) [[unlikely]] {
        fmt::println(stderr, "sdl create window error:{}", SDL_GetError());
        return -1;
    }

    SDL_SetWindowAlwaysOnTop(win.get(), true);

    EventQueue queue{};
    // start message pump for hook
    const auto displayEvent = SDL_RegisterEvents(2);
    const auto hideEvent = displayEvent + 1;
    auto hookThread = std::jthread(llHookRun,
                                   HookContext{ .queue = &queue,
                                                .displayEvent = displayEvent,
                                                .hideEvent = hideEvent });

    // start render loop
    auto guiThread = std::jthread(runGui,
                                  GuiContext{ .queue = &queue,
                                              .window = win.get() });

    // init tray icon
    auto tray = MakeUniqueWith<SDL_DestroyTray>(
        SDL_CreateTray(nullptr, "fantacyZone"));

    if (!tray) {
        fmt::println(stderr, "sdl create tray error:{}", SDL_GetError());
        return -1;
    }

    auto menu = SDL_CreateTrayMenu(tray.get());
    auto quitEntry =
        SDL_InsertTrayEntryAt(menu, -1, "quit", SDL_TRAYENTRY_BUTTON);

    SDL_SetTrayEntryCallback(
        quitEntry,
        [](void *userdata, SDL_TrayEntry *entry) -> void {
            SDL_Event quitEvent{ .type = SDL_EventType::SDL_EVENT_QUIT };
            SDL_PushEvent(&quitEvent);
        },
        nullptr);

    // start event loop
    SDL_Event event{};
    while (true) {
        SDL_WaitEvent(&event);

        if (event.type == SDL_EventType::SDL_EVENT_QUIT) [[unlikely]] {
            break;
        } else if (event.type == displayEvent) {
            SDL_ShowWindow(win.get());
        } else if (event.type == hideEvent) {
            SDL_HideWindow(win.get());
        }
    }

    return 0;
}
