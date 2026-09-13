#pragma once

#include <stop_token>
#include <SDL3/SDL.h>
#include <lockfree/spsc/queue.hpp>

#include "../helper/event.hpp"

struct HookContext {
    EventQueue *queue;
    uint32_t displayEvent;
    uint32_t hideEvent;
};

void llHookRun(std::stop_token token, HookContext ctx) noexcept;