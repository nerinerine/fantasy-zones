#include <thread>

#include <fmt/base.h>
#include <SDL3/SDL.h>

#include "define.hpp"
#include "helper/sdl_ptr_factory.hpp"
#include "desktop_info/info.hpp"
#include "helper/showWinInMousePos.hpp"
#include "grid/grid.hpp"
#include "llhook/hook.hpp"

auto main(int argc, char **argv) -> int {
    std::setlocale(LC_ALL,
                   ""); //使用用户环境默认locale以使fmt可以正常打印宽字符串

    // init sdl
    if (!SDL_Init(SDL_INIT_VIDEO)) {
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

    if (!win) {
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

    if (!rdr) {
        fmt::println(stderr, "sdl create render error:{}", SDL_GetError());
        return -1;
    }

    SDL_SetRenderDrawBlendMode(rdr.get(), SDL_BLENDMODE_BLEND);

    // load config
    const auto config = loadConfig("setting.ini");

    // main loop
    SDL_Event event{};
    bool shouldQuit = false;
    bool isWindowShow = false;

    while (!shouldQuit) {
        for (auto event = queue.Pop(); event.has_value(); event = queue.Pop()) {
            event.and_then([&](Event e) -> std::optional<Event> {
                switch (e) {
                case Event::WIN_DISPLAY:
                    showWinInMousePos(win.get());
                    isWindowShow = true;
                    break;
                case Event::WIN_HIDE:
                    SDL_HideWindow(win.get());
                    isWindowShow = false;
                    break;
                case Event::THREAD_STOP:
                    shouldQuit = true;
                    break;
                default:
                    break;
                }
                return std::nullopt;
            });
        }

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EventType::SDL_EVENT_QUIT) {
                shouldQuit = true;
            }
        }

        if (!isWindowShow) {
            std::this_thread::sleep_for(SLEEP_DURATION);
            continue;
        }

        for (const auto &grid : config) {
            grid.draw(rdr.get());
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
