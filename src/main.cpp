#include <thread>
#include <clocale>

#include <fmt/base.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_tray.h>

#include "gui/gui.hpp"
#include "llhook/hook.hpp"
#include "helper/make_unique_with.hpp"

auto main(int argc, char **argv) -> int {
    std::setlocale(LC_ALL,
                   ""); //使用用户环境默认locale以使fmt可以正常打印宽字符串

#ifdef DEBUG
    fmt::println("debug mode");
#endif

    // init sdl
    if (!SDL_Init(SDL_INIT_VIDEO)) [[unlikely]] {
        fmt::println(stderr, "sdl init error:{}", SDL_GetError());
        return -1;
    }

    const auto SDLModelHandle = MakeUniqueWith<SDL_Quit>();

    EventQueue queue{};
    // start event loop for hook
    auto hookThread = std::jthread(LLHook::run, &queue);

    // init tray icon

    // auto tray = MakeUniqueWith<SDL_DestroyTray>(
    //     SDL_CreateTray(nullptr, "fantacyZone"));

    // if (!tray) {
    //     fmt::println(stderr, "sdl create tray error:{}", SDL_GetError());
    //     return -1;
    // }

    // auto menu = SDL_CreateTrayMenu(tray.get());
    // auto quitEntry =
    //     SDL_InsertTrayEntryAt(menu, -1, "quit", SDL_TRAYENTRY_BUTTON);

    // auto trayData = std::make_tuple(&queue, &hookThread);
    // using TrayData_t = decltype(trayData);

    // SDL_SetTrayEntryCallback(
    //     quitEntry,
    //     [](void *userdata, SDL_TrayEntry *entry) -> void {
    //         const auto trayData = static_cast<TrayData_t *>(userdata);
    //         auto [queue, hookThread] = *trayData;
    //         hookThread->request_stop();
    //     },
    //     &trayData);

    // start event loop for gui
    auto guiThread = std::jthread(runGUi, &queue);

    return 0;
}
