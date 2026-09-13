#pragma once

#include <stop_token>
#include <lockfree/spsc/queue.hpp>

void llHookRun(std::stop_token token) noexcept;