#include <thread>

#include "fmt/core.h"
#include "SDL3/SDL.h" // IWYU pragma: keep

#include "helper/sdl_ptr_factory.hpp"
#include "define.hpp"
#include "helper/screen.hpp"

auto main(int argc, char **argv) -> int {
    // init sdl
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fmt::print("sdl init error:{}", SDL_GetError());
        return -1;
    }

    const auto SDLModelHandle = SDLPtrFactoryGen<SDL_Quit>;

    // create window
    const auto screenSize = getScreenSize();

    const auto win = SDLPtrFactoryGen<SDL_DestroyWindow>(
        SDL_CreateWindow("Fantacy Zone",
                         screenSize.width / 4,
                         screenSize.height / 4,
                         SDL_WINDOW_BORDERLESS));

    if (!win) {
        fmt::print("sdl create window error:{}", SDL_GetError());
        return -1;
    }

    // create render
    const auto rdr = SDLPtrFactoryGen<SDL_DestroyRenderer>(
        SDL_CreateRenderer(win.get(), nullptr));

    if (!rdr) {
        fmt::print("sdl create render error:{}", SDL_GetError());
        return -1;
    }

    // main loop
    const auto renderLoop = [&]() -> void {
        SDL_Event event{};

        while (true) {
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_EventType::SDL_EVENT_QUIT) {
                    return;
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
    };
    renderLoop();

    return 0;
}
