#pragma once

#include "lockfree/spsc/queue.hpp"
#include "../event.hpp"
#include <stop_token>

struct LLHook {
    static constexpr int QUEUE_SIZE = 256;
    using EventQueue = lockfree::spsc::Queue<Event, QUEUE_SIZE>;
    static auto init() noexcept -> bool;
    static void run(std::stop_token token,EventQueue* queue) noexcept;
    static void destory() noexcept;
};