#include"debug_print_window_title.hpp"

#include <array>
#include <fmt/format.h>
#include <fmt/xchar.h>

void printWindowTitle(HWND window){
    constexpr size_t BUF_SIZE = 256;
    std::array<wchar_t,BUF_SIZE> buf{};
    GetWindowTextW(window,buf.data(),BUF_SIZE);
    fmt::println(L"{}",buf.data());
}