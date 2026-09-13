#include "gui.hpp"

#include <SDL3/SDL.h>
#include <thread>

#include "../define.hpp"
#include "../grid/grid.hpp"
#include "../helper/make_unique_with.hpp"
#include "../helper/set_point_window_pos.hpp"
#include "../desktop_info/info.hpp"

struct EventHandle {
    const GridItem *activeGrid = nullptr;
    bool shouldQuit = false;
    SDL_Point winPos{};

    void operator()(auto &e) {
        e();
    }
    void operator()(EventQuit &) {
        shouldQuit = true;
    }
    void operator()(EventSubmit &) {
        if (activeGrid) {
            setPointWindowPos(activeGrid);
        }
    }
};

auto runGui(const GuiContext ctx) -> int {
    const auto queue = ctx.queue;

    // create render
    const auto rdr = MakeUniqueWith<SDL_DestroyRenderer>(
        SDL_CreateRenderer(ctx.window, nullptr));

    if (!rdr) [[unlikely]] {
        fmt::println(stderr, "sdl create render error:{}", SDL_GetError());
        return -1;
    }

    SDL_SetRenderDrawBlendMode(rdr.get(), SDL_BLENDMODE_BLEND);

    // load config
    const auto config = loadConfig("./setting.json");

    // main loop

    EventHandle eventHandle{};

    while (!eventHandle.shouldQuit) [[likely]] {
        // 来自hook线程的事件

        while (!queue->empty()) {
            auto &e = queue->front();
            std::visit(eventHandle, e);
            queue->pop();
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