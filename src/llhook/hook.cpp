#include "hook.hpp"

#include "fmt/core.h"

#include <Windows.h>

class InitConfig {
  public:
    bool init = false;
    HHOOK kbHook{};

    [[nodiscard]] static auto instance() noexcept -> InitConfig * {
        static InitConfig config{};
        return &config;
    }

  private:
    InitConfig() = default;
};

auto CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
    -> LRESULT {
    if (nCode == HC_ACTION) {
        auto pKeyBoard = std::bit_cast<KBDLLHOOKSTRUCT *>(lParam);
        if (pKeyBoard->vkCode == VK_LMENU) {
            if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
                fmt::println("press alt");
            }
            if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
                fmt::println("release alt");
            }
        }
    }

    return CallNextHookEx(InitConfig::instance()->kbHook,
                          nCode,
                          wParam,
                          lParam);
}

auto LLHook::init() noexcept -> bool {
    auto config = InitConfig::instance();

    if (config->init) {
        return true;
    }

    config->kbHook = SetWindowsHookEx(WH_KEYBOARD_LL,       //hook 目标
                                      LowLevelKeyboardProc, // 回调
                                      nullptr,              //意义不明的参数
                                      0                     //0表示全局hook
    );

    if (!config->kbHook) {
        fmt::println("set hook error{}", GetLastError());
        return false;
    }

    config->init = true;
    return true;
}

void LLHook::run() noexcept {
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void LLHook::destory() noexcept {
    auto config = InitConfig::instance();
    UnhookWindowsHookEx(config->kbHook);
    config->init = false;
}