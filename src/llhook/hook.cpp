#include "hook.hpp"

#include <thread>
#include <fmt/base.h>

#include "../define.hpp"

#include <Windows.h>

class Config {
  public:
    HHOOK kbHook{};
    HWINEVENTHOOK winMoveHook{};
    LLHook::EventQueue *queue = nullptr;
    bool isWindowMoving = false;
    bool isAltkeyPushing = false;
    bool init = false;

    [[nodiscard]] static auto instance() noexcept -> Config * {
        static Config config{};
        return &config;
    }

  private:
    Config() = default;
};

void submitWindowEvent() noexcept {
    const auto config = Config::instance();
    if (!config->isWindowMoving || !config->isAltkeyPushing) {
        config->queue->Push(Event::WIN_HIDE);
    } else {
        config->queue->Push(Event::WIN_DISPLAY);
    }
}

auto CALLBACK altCheckCallback(int nCode, WPARAM wParam, LPARAM lParam)
    -> LRESULT {
    const auto queue = Config::instance()->queue;
    if (nCode == HC_ACTION) {
        auto pKeyBoard = std::bit_cast<KBDLLHOOKSTRUCT *>(lParam);
        if (pKeyBoard->vkCode == VK_LMENU) {
            if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
                Config::instance()->isAltkeyPushing = true;
            }
            if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
                Config::instance()->isAltkeyPushing = false;
            }
            submitWindowEvent();
        }

        // 这是暂时的，毕竟总得有个正常退出的方法
        if (pKeyBoard->vkCode == *"C") {
            queue->Push(Event::THREAD_STOP);
        }
    }

    return CallNextHookEx(Config::instance()->kbHook, nCode, wParam, lParam);
}
// NOLINTBEGIN(bugprone-easily-swappable-parameters)
// win32api底层逻辑这一块...哦对了,
void CALLBACK winEventProc(HWINEVENTHOOK hWinEventHook,
                           DWORD event,
                           HWND hwnd,
                           LONG idObject,
                           LONG idChild,
                           DWORD idEventThread,
                           DWORD dwmsEventTime) {
    // NOLINTEND(bugprone-easily-swappable-parameters)
    if (event == EVENT_SYSTEM_MOVESIZESTART) {
        Config::instance()->isWindowMoving = true;
    }
    if (event == EVENT_SYSTEM_MOVESIZEEND) {
        Config::instance()->isWindowMoving = false;
    }
    submitWindowEvent();
}

auto LLHook::init(LLHook::EventQueue *queue) noexcept -> bool {
    auto config = Config::instance();

    if (config->init) {
        return true;
    }

    config->kbHook = SetWindowsHookEx(WH_KEYBOARD_LL,   //hook 目标
                                      altCheckCallback, // 回调
                                      nullptr,          //意义不明的参数
                                      0                 //0表示全局hook
    );

    if (!config->kbHook) {
        fmt::println("set hook error{}", GetLastError());
        return false;
    }

    config->winMoveHook = SetWinEventHook(EVENT_SYSTEM_MOVESIZESTART,
                                          EVENT_SYSTEM_MOVESIZEEND,
                                          nullptr,
                                          winEventProc,
                                          0,
                                          0,
                                          WINEVENT_OUTOFCONTEXT |
                                              WINEVENT_SKIPOWNPROCESS);

    if (!config->winMoveHook) {
        fmt::println("set win event hook error{}", GetLastError());
        return false;
    }

    config->queue = queue;

    config->init = true;
    return true;
}

// NOLINTNEXTLINE(performance-unnecessary-value-param) 有什么话和标准库说去吧
void LLHook::run(std::stop_token token) noexcept {
    MSG msg;
    while (!token.stop_requested()) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            // 其实这个检测应该没什么用，但是D指导这么写了...(目移)
            if (msg.message == WM_QUIT)
                break;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            // 暂且借用一下渲染模块那边的sleep duration
            // 不过感觉其实不是很需要再改就是了
            std::this_thread::sleep_for(SLEEP_DURATION);
        }
    }
}

void LLHook::destory() noexcept {
    auto config = Config::instance();
    UnhookWindowsHookEx(config->kbHook);
    UnhookWinEvent(config->winMoveHook);
    config->init = false;
}