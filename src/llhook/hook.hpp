#pragma once

#include <stop_token>
#include <lockfree/spsc/queue.hpp>

#include "../helper/event.hpp"

void llHookRun(std::stop_token token, EventQueue *const queue) noexcept;