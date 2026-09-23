#include <thread>
#include <clocale>

#include <fmt/base.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_tray.h>
#include <etl/singleton.h>

#include "grid/grid.hpp"
#include "gui/gui.hpp"
#include "llhook/hook.hpp"

#include "helper/events.hpp"
#include "helper/make_unique_with.hpp"

#include <windows.h>
#include <Psapi.h>

constexpr auto getEvents = etl::singleton<Events>::instance;

// NOLINTNEXTLINE:fuck u micorsoft
auto main(int argc, char **argv) -> int {
    // int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow){
    std::setlocale(LC_ALL,
                   ""); //使用用户环境默认locale以使fmt可以正常打印宽字符串

    // init sdl
    if (!SDL_Init(SDL_INIT_VIDEO)) [[unlikely]] {
        fmt::println(stderr, "sdl init error:{}", SDL_GetError());
        return -1;
    }

    const auto SDLModelHandle = MakeUniqueWith<SDL_Quit>();

    // create window
    SDL_SetHint(SDL_HINT_WINDOW_ACTIVATE_WHEN_SHOWN, "0");

    const auto win = MakeUniqueWith<SDL_DestroyWindow>(
        SDL_CreateWindow("Fantacy Zone",
                         0,
                         0,
                         SDL_WINDOW_BORDERLESS | SDL_WINDOW_HIDDEN |
                             SDL_WINDOW_ALWAYS_ON_TOP));

    if (!win) [[unlikely]] {
        fmt::println(stderr, "sdl create window error:{}", SDL_GetError());
        return -1;
    }

    SDL_SetWindowAlwaysOnTop(win.get(), true);

    // register custom event
    const auto RegistedEvents = SDL_RegisterEvents(3);
    if (!RegistedEvents) [[unlikely]] {
        fmt::println(stderr, "sdl register events error:{}", SDL_GetError());
        return -1;
    }
    etl::singleton<Events>::create(
        Events{ .displayEvent = { RegistedEvents },
                .submitEvent = { .type = RegistedEvents + 1 },
                .cancelEvent = { .type = RegistedEvents + 2 } });

    // start message pump for hook
    auto hookThread = std::jthread(llHookRun);

    // create render
    const auto rdr = MakeUniqueWith<SDL_DestroyRenderer>(
        SDL_CreateRenderer(win.get(), nullptr));

    if (!rdr) [[unlikely]] {
        fmt::println(stderr, "sdl create render error:{}", SDL_GetError());
        return -1;
    }

    SDL_SetRenderDrawBlendMode(rdr.get(), SDL_BLENDMODE_BLEND);

    // load config
    const auto config = loadConfig("./setting.json");

    // init tray icon
    auto tray = MakeUniqueWith<SDL_DestroyTray>(
        SDL_CreateTray(nullptr, "fantacyZone"));

    if (!tray) [[unlikely]] {
        fmt::println(stderr, "sdl create tray error:{}", SDL_GetError());
        return -1;
    }

    const auto menu = SDL_CreateTrayMenu(tray.get());
    const auto quitEntry =
        SDL_InsertTrayEntryAt(menu, -1, "quit", SDL_TRAYENTRY_BUTTON);

    SDL_SetTrayEntryCallback(
        quitEntry,
        [](void *userdata, SDL_TrayEntry *entry) -> void {
            const auto hookThread = static_cast<std::jthread *>(userdata);
            hookThread->request_stop();
            SDL_Event quitEvent{ .type = SDL_EventType::SDL_EVENT_QUIT };
            SDL_PushEvent(&quitEvent);
        },
        &hookThread);

    // start event loop
    SDL_Event event{};
    const GuiContext guiCtx{ .rdr = rdr.get(),
                             .win = win.get(),
                             .list = &config };

    EmptyWorkingSet(GetCurrentProcess());

    while (true) {
        SDL_WaitEvent(&event);

        if (event.type == SDL_EventType::SDL_EVENT_QUIT) [[unlikely]] {
            break;
        } else if (event.type == getEvents().displayEvent.type) [[likely]] {
            runGui(guiCtx);
        }
    }
    return 0;
}
