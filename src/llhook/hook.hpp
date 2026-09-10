#pragma once

#include <stop_token>
#include <lockfree/spsc/queue.hpp>

#include "../helper/event.hpp"

struct LLHook {
    static void run(std::stop_token token, EventQueue *const queue) noexcept;
};