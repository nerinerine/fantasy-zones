#pragma once
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

constexpr auto WINDOW_WIDTH_SCALE = 0.25;
constexpr auto WINDOW_HEIGHT_SCALE = 0.25;