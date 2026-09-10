#ifndef DEBUG
#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#endif

#include <thread>

#include <fmt/base.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_tray.h>

#include "define.hpp"
#include "helper/make_unique_with.hpp"
#include "desktop_info/info.hpp"
#include "helper/showWinInMousePos.hpp"
#include "grid/grid.hpp"
#include "llhook/hook.hpp"
#include "helper/set_point_window_pos.hpp"

struct EventHandle {
    SDL_Window *win;
    const GridItem *activeGrid = nullptr;
    bool shouldQuit = false;
    SDL_Point winPos{};

    void operator()(auto &e) {
        e();
    }
    void operator()(EventQuit &) {
        shouldQuit = true;
    }
    void operator()(EventDisplay &) {
        showWinInMousePos(win);
        SDL_GetWindowPosition(win, &winPos.x, &winPos.y);
    }
    void operator()(EventSubmit &) {
        SDL_HideWindow(win);

        if (activeGrid) {
            setPointWindowPos(activeGrid);
        }
    }
    void operator()(EventCancel &) {
        SDL_HideWindow(win);
    }
};

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

    // start event loop for hook
    EventQueue queue{};
    auto hookThread = std::jthread(LLHook::run, &queue);

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

    // main loop
    SDL_Event event{};

    EventHandle eventHandle{ .win = win.get() };

    while (!eventHandle.shouldQuit) [[likely]] {
        // 来自hook线程的事件

        while (!queue.empty()) {
            auto &e = queue.front();
            std::visit(eventHandle, e);
            queue.pop();
        }

        // SDL自身的事件
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EventType::SDL_EVENT_QUIT) [[unlikely]] {
                eventHandle.shouldQuit = true;
            }
        }

        const SDL_FPoint mouse_relPos{
            .x = static_cast<float>(getMousePos().x - eventHandle.winPos.x),
            .y = static_cast<float>(getMousePos().y - eventHandle.winPos.y)
        };

        eventHandle.activeGrid = nullptr;
        for (const auto &grid : config) {
            if (grid.visible) [[unlikely]] {
                grid.hitRect.drawBorder(grid.col, rdr.get());
                grid.hitRect.drawInner(grid.col, rdr.get());
            }

            const auto checkRect = grid.hitRect.outerDisplayRect();
            if (!eventHandle.activeGrid &&
                SDL_PointInRectFloat(&mouse_relPos, &checkRect)) [[unlikely]] {
                eventHandle.activeGrid = &grid;
            }
        }

        if (eventHandle.activeGrid) {
            const DRect &activeRect = eventHandle.activeGrid->targetRect;
            const SDL_Color col{ .r = 0x00, .g = 0xff, .b = 0xcc, .a = 0xff };
            activeRect.drawBorder(col, rdr.get());
            activeRect.drawShadow(col, rdr.get());
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
