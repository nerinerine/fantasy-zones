#include "gen_color.hpp"
#include <cassert>
#include <cstdlib>

constexpr double HUE_OFFSET = 21;
constexpr double SAT = 0.4;
constexpr double VAL = 1;

struct HSVColor {
    double h{};
    double s{};
    double v{};
};

auto mapHSV(const HSVColor &color) noexcept -> SDL_Color {
    const auto [h, s, v] = color;

    assert(0 <= h && h < 360);
    assert(0 <= s && s <= 1);
    assert(0 <= v && v <= 1);

    using namespace std;

    const auto c = v * s;
    const auto x = c * (1 - abs(fmod(h / 60, 2) - 1));
    const auto m = v - c;
    const auto h_pos = static_cast<int>(h / 60);
    double r1{}, g1{}, b1{};
    switch (h_pos) {
    case 0:
        r1 = c, g1 = x;
        break;
    case 1:
        r1 = x, g1 = c;
        break;
    case 2:
        g1 = c, b1 = x;
        break;
    case 3:
        g1 = x, b1 = c;
        break;
    case 4:
        r1 = x, b1 = c;
        break;
    default:
        r1 = c, b1 = x;
        break;
    }
    const auto r = static_cast<uint8_t>((r1 + m) * 255.0);
    const auto g = static_cast<uint8_t>((g1 + m) * 255.0);
    const auto b = static_cast<uint8_t>((b1 + m) * 255.0);
    return { .r = r, .g = g, .b = b, .a = 1 };
}

auto genColor() noexcept -> SDL_Color {
    static double hue = 0;
    const auto col = mapHSV({ .h = hue, .s = SAT, .v = VAL });
    hue += HUE_OFFSET;
    return col;
}