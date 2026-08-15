#pragma once
#include <cstdint>

enum class Event:uint8_t{
    WIN_DISPLAY,
    WIN_HIDE,
    THREAD_PAUSE,
    THREAD_RUN,
};