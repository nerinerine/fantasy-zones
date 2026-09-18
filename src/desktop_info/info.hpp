#pragma once
struct Rect {
    int w;
    int h;
};

auto getScreenSize() -> Rect;
auto getWindowSize(Rect size) -> Rect;

struct Pos {
    int x;
    int y;
};

auto getScreenStartPoint()->Pos;
auto getMousePos() -> Pos;

struct RelPos {
    double x;
    double y;
};

auto operator+(const Pos &pos, const Rect &rect) -> Pos;
auto operator/(const Pos &pos, const Rect &rect) -> RelPos;
auto operator==(const Rect &lhs, const Rect &rhs) -> bool;
inline auto operator!=(const Rect &lhs, const Rect &rhs) -> bool {
    return !(lhs == rhs);
}