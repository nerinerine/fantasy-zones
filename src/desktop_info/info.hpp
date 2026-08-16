#pragma once
struct Rect {
    int width;
    int height;
};

auto getScreenSize() -> Rect;
auto getWindowSize(Rect size) ->Rect;

struct Pos {
    int x;
    int y;
};

auto getMousePos() -> Pos;

struct RelPos {
    double x;
    double y;
};

auto operator/(Pos pos, Rect rect) -> RelPos;