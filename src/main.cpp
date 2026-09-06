#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#include <thread>

#include <fmt/base.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_tray.h>

#include "define.hpp"
#include "helper/sdl_ptr_factory.hpp"
#include "desktop_info/info.hpp"
#include "helper/showWinInMousePos.hpp"
#include "grid/grid.hpp"
#include "llhook/hook.hpp"
#include "helper/set_point_window_pos.hpp"

auto main(int argc, char **argv) -> int {
    std::setlocale(LC_ALL,
                   ""); //使用用户环境默认locale以使fmt可以正常打印宽字符串

    // init sdl
    if (!SDL_Init(SDL_INIT_VIDEO)) [[unlikely]] {
        fmt::println(stderr, "sdl init error:{}", SDL_GetError());
        return -1;
    }

    const auto SDLModelHandle = SDLPtrFactoryGen<SDL_Quit>();

    // create window
    const auto windowSize = getWindowSize(getScreenSize());

    const auto win = SDLPtrFactoryGen<SDL_DestroyWindow>(
        SDL_CreateWindow("Fantacy Zone",
                         windowSize.w,
                         windowSize.h,
                         SDL_WINDOW_BORDERLESS | SDL_WINDOW_HIDDEN));

    if (!win) [[unlikely]] {
        fmt::println(stderr, "sdl create window error:{}", SDL_GetError());
        return -1;
    }

    SDL_SetWindowAlwaysOnTop(win.get(), true);

    // reg low level hook
    LLHook::EventQueue queue{};
    LLHook::init(&queue);
    auto llhookHandle = SDLPtrFactoryGen<LLHook::destory>();

    // start event loop for hook
    auto hookThread = std::jthread(LLHook::run);

    // create render
    const auto rdr = SDLPtrFactoryGen<SDL_DestroyRenderer>(
        SDL_CreateRenderer(win.get(), nullptr));

    if (!rdr) [[unlikely]] {
        fmt::println(stderr, "sdl create render error:{}", SDL_GetError());
        return -1;
    }

    SDL_SetRenderDrawBlendMode(rdr.get(), SDL_BLENDMODE_BLEND);

    // load config
    const auto config = loadConfig("setting.ini");

    // init tray icon
    bool shouldQuit = false;

    auto tray = SDLPtrFactoryGen<SDL_DestroyTray>(
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
            bool *shouldQuit = static_cast<bool *>(userdata);
            *shouldQuit = true;
        },
        &shouldQuit);

    // main loop
    SDL_Event event{};
    SDL_Point WinPos{};
    const GridItem *activeGrid = nullptr;
    bool isWindowShow = false;

    while (!shouldQuit) [[likely]] {
        // 来自hook线程的事件
        for (auto event = queue.Pop(); event.has_value(); event = queue.Pop()) {
            event.and_then([&](Event e) -> std::optional<Event> {
                switch (e) {
                case Event::WIN_DISPLAY:
                    WinPos = showWinInMousePos(win.get());
                    isWindowShow = true;
                    break;
                case Event::WIN_HIDE:
                    SDL_HideWindow(win.get());

                    if (isWindowShow && activeGrid) {
                        setPointWindowPos(activeGrid);
                    }

                    isWindowShow = false;
                    break;
                default:
                    break;
                }
                return std::nullopt;
            });
        }
        // SDL自身的事件
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EventType::SDL_EVENT_QUIT) [[unlikely]] {
                shouldQuit = true;
            }
        }
        // 窗口未显示时跳过渲染
        if (!isWindowShow) [[likely]] {
            std::this_thread::sleep_for(SLEEP_DURATION);
            continue;
        }

        const SDL_FPoint mouse_relPos{
            .x = static_cast<float>(getMousePos().x - WinPos.x),
            .y = static_cast<float>(getMousePos().y - WinPos.y)
        };

        activeGrid = nullptr;
        for (const auto &grid : config) {
            grid.draw(rdr.get());

            const auto checkRect = grid.outerDisplayRect();
            if (SDL_PointInRectFloat(&mouse_relPos, &checkRect)) {
                activeGrid = &grid;
                const GridItem activeGrid{ .x = grid.x,
                                           .y = grid.y,
                                           .w = grid.w,
                                           .h = grid.h,
                                           .col = SDL_Color{ .r = 0x00,
                                                             .g = 0xff,
                                                             .b = 0xcc,
                                                             .a = 0xff } };
                activeGrid.drawBorder(rdr.get());
                activeGrid.drawShadow(rdr.get());
            }
        }

        SDL_RenderPresent(rdr.get());

        SDL_SetRenderDrawColor(rdr.get(),
                               CLEAR_COLOR.r,
                               CLEAR_COLOR.g,
                               CLEAR_COLOR.b,
                               CLEAR_COLOR.a);
        SDL_RenderClear(rdr.get());

        std::this_thread::sleep_for(SLEEP_DURATION);
    }

    return 0;
}
