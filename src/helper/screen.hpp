#pragma once
struct rect {
    int width;
    int height;
};

auto getScreenSize() -> rect;