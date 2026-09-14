#include "gui.hpp"

#include <thread>

#include <fmt/format.h>
#include <SDL3/SDL.h>
#include <etl/singleton.h>

#include "../grid/grid.hpp"
#include "../desktop_info/info.hpp"

#include "../helper/events.hpp"
#include "../helper/set_point_window_pos.hpp"
#include "../helper/show_win_in_mouse_pos.hpp"

#include <chrono>
#include <cstdint>
using namespace std::literals;

constexpr char FPS{ 30 };
constexpr auto SLEEP_DURATION{ 1000ms / FPS };

constexpr struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} CLEAR_COLOR{ .r = 0xff, .g = 0xff, .b = 0xff, .a = 0xff };

constexpr auto getEvents = etl::singleton<Events>::instance;

void runGui(const GuiContext ctx) {
    const GridItem *activeGrid{};

    SDL_Event event{};
    SDL_Point winPos = showWinInMousePos(ctx.win);

    // main loop
    while (true) {
        // 接管sdl事件循环
        while (SDL_PollEvent(&event)) [[unlikely]] {
            if (event.type == getEvents().submitEvent.type) {
                SDL_HideWindow(ctx.win);
                if (activeGrid) {
                    setPointWindowPos(activeGrid);
                }
                return;
            } else if (event.type == getEvents().cancelEvent.type) {
                SDL_HideWindow(ctx.win);
                return;
            } else if (event.type == SDL_EventType::SDL_EVENT_QUIT)
                [[unlikely]] {
                // 在退出之前把消耗的quit重新推回队列
                SDL_Event quit{ .type = SDL_EventType::SDL_EVENT_QUIT };
                SDL_PushEvent(&quit);
                return;
            }
        }

        const SDL_FPoint mouse_relPos{
            .x = static_cast<float>(getMousePos().x - winPos.x),
            .y = static_cast<float>(getMousePos().y - winPos.y)
        };

        activeGrid = nullptr;
        for (const auto &grid : *ctx.list) {
            if (grid.visible) {
                grid.hitRect.drawBorder(grid.col, ctx.rdr);
                grid.hitRect.drawInner(grid.col, ctx.rdr);
            }

            const auto checkRect = grid.hitRect.outerDisplayRect();
            if (!activeGrid && SDL_PointInRectFloat(&mouse_relPos, &checkRect))
                [[unlikely]] {
                activeGrid = &grid;
            }
        }

        if (activeGrid) {
            const DRect &activeRect = activeGrid->targetRect;
            const SDL_Color col{ .r = 0x00, .g = 0xff, .b = 0xcc, .a = 0xff };
            activeRect.drawBorder(col, ctx.rdr);
            activeRect.drawShadow(col, ctx.rdr);
        }

        SDL_RenderPresent(ctx.rdr);

        SDL_SetRenderDrawColor(ctx.rdr,
                               CLEAR_COLOR.r,
                               CLEAR_COLOR.g,
                               CLEAR_COLOR.b,
                               CLEAR_COLOR.a);
        SDL_RenderClear(ctx.rdr);

        std::this_thread::sleep_for(SLEEP_DURATION);
    }
}