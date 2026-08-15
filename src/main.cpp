#include <thread>

#include <fmt/core.h>
#include <SDL3/SDL.h>

#include "define.hpp"
#include "helper/sdl_ptr_factory.hpp"
#include "helper/screen.hpp"
#include "llhook/hook.hpp"

auto main(int argc, char **argv) -> int {
    // init sdl
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fmt::println(stderr, "sdl init error:{}", SDL_GetError());
        return -1;
    }

    const auto SDLModelHandle = SDLPtrFactoryGen<SDL_Quit>();

    // create window
    const auto screenSize = getScreenSize();
    const auto winWidth = static_cast<int>(screenSize.width *
                                           WINDOW_WIDTH_SCALE);
    const auto winHeight = static_cast<int>(screenSize.height *
                                            WINDOW_HEIGHT_SCALE);

    const auto win = SDLPtrFactoryGen<SDL_DestroyWindow>(SDL_CreateWindow(
        "Fantacy Zone", winWidth, winHeight, SDL_WINDOW_BORDERLESS));

    if (!win) {
        fmt::println(stderr, "sdl create window error:{}", SDL_GetError());
        return -1;
    }

    SDL_SetWindowAlwaysOnTop(win.get(), true);

    // reg low level hook
    LLHook::init();
    auto llhookHandle = SDLPtrFactoryGen<LLHook::destory>();

    // start event loop for hook
    LLHook::EventQueue queue{};
    auto hookThread = std::jthread(LLHook::run, &queue);

    // create render
    const auto rdr = SDLPtrFactoryGen<SDL_DestroyRenderer>(
        SDL_CreateRenderer(win.get(), nullptr));

    if (!rdr) {
        fmt::println(stderr, "sdl create render error:{}", SDL_GetError());
        return -1;
    }

    // main loop
    SDL_Event event{};
    bool shouldQuit = false;

    while (!shouldQuit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EventType::SDL_EVENT_QUIT) {
                shouldQuit = true;
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
