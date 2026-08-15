#pragma once

struct LLHook{
    static auto init() noexcept -> bool;
    static void run() noexcept;
    static void destory() noexcept;
};