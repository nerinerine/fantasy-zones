#pragma once
#include <cstdint>

enum class Event : uint8_t {
    WIN_DISPLAY,
    WIN_SUBMIT,
    WIN_CANCLE,
    THREAD_PAUSE,
    THREAD_RUN,
};