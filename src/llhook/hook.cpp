#include "hook.hpp"

#include <SDL3/SDL.h>
#include <thread>
#include <fmt/base.h>

#include "../define.hpp"
#include "../helper/make_unique_with.hpp"

#include <Windows.h>

class KeyRecord {
  public:
    KeyRecord(const KeyRecord &) = delete;
    KeyRecord(KeyRecord &&) = delete;
    auto operator=(const KeyRecord &) = delete;
    auto operator=(KeyRecord &&) = delete;

    [[nodiscard]] static auto i() noexcept -> KeyRecord & {
        static KeyRecord inst{};
        return inst;
    }

    bool isWindowMoving = false;
    bool isAltkeyPushing = false;

  private:
    KeyRecord() = default;
    ~KeyRecord() = default;
};

class Hooks {
  public:
    Hooks(const Hooks &) = delete;
    Hooks(Hooks &&) = delete;
    auto operator=(const Hooks &) = delete;
    auto operator=(Hooks &&) = delete;

    HHOOK kbHook{};
    HWINEVENTHOOK winMoveHook{};

    [[nodiscard]] static auto i() noexcept -> Hooks & {
        static Hooks inst{};
        return inst;
    }

  private:
    Hooks() = default;
    ~Hooks() = default;
};

class Context {
  public:
    Context(const Context &) = delete;
    Context(Context &&) = delete;
    auto operator=(const Context &) = delete;
    auto operator=(Context &&) = delete;

    EventQueue *queue = nullptr;
    std::latch *pauseToken = nullptr;

    [[nodiscard]] static auto i() noexcept -> Context & {
        static Context inst{};
        return inst;
    }

  private:
    Context() = default;
    ~Context() = default;
};

void submitWindowEvent() noexcept {
    static auto &ctx = Context::i();
    static const auto queue = ctx.queue;
    static const auto &kb = KeyRecord::i();

    if (kb.isWindowMoving && kb.isAltkeyPushing && ctx.pauseToken) {
        // 满足激活条件，且处在暂停状态
        ctx.pauseToken->count_down();
        ctx.pauseToken = nullptr;
        queue->emplace(EventDisplay{});
    } else if (!ctx.pauseToken && !kb.isWindowMoving) {
        // 处在激活状态时停止移动窗口
        EventPause pause{};
        ctx.pauseToken = pause.token();
        queue->emplace(EventSubmit{});
        queue->emplace(std::move(pause));
    } else if (!ctx.pauseToken && !kb.isAltkeyPushing) {
        // 处在激活状态时松开alt
        EventPause pause{};
        ctx.pauseToken = pause.token();
        queue->emplace(EventCancel{});
        queue->emplace(std::move(pause));
    } //else 什么都不做
}

auto CALLBACK keyboardLLEventProc(int nCode, WPARAM wParam, LPARAM lParam)
    -> LRESULT {
    static auto &kb = KeyRecord::i();
    if (nCode == HC_ACTION) {
        const auto pKeyBoard = std::bit_cast<KBDLLHOOKSTRUCT *>(lParam);
        if (pKeyBoard->vkCode == VK_LMENU) [[unlikely]] {
            if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
                kb.isAltkeyPushing = true;
            }
            if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
                kb.isAltkeyPushing = false;
            }
            submitWindowEvent();
        }
    }

    return CallNextHookEx(Hooks::i().kbHook, nCode, wParam, lParam);
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
    static auto &kb = KeyRecord::i();
    if (event == EVENT_SYSTEM_MOVESIZESTART) {
        kb.isWindowMoving = true;
    }
    if (event == EVENT_SYSTEM_MOVESIZEEND) {
        kb.isWindowMoving = false;
    }
    submitWindowEvent();
}

void destory() noexcept;
// 我们应当假设init函数在所有函数之前被调用且仅调用一次
auto init(EventQueue *const queue) -> bool {
    static bool isInit = false;
    if (isInit) {
        return false;
    }

    auto &ctx = Context::i();
    auto &hooks = Hooks::i();

    hooks.kbHook = SetWindowsHookEx(WH_KEYBOARD_LL,      //hook 目标
                                    keyboardLLEventProc, // 回调
                                    nullptr,             //意义不明的参数
                                    0                    //0表示全局hook
    );

    if (!hooks.kbHook) [[unlikely]] {
        fmt::println(stderr, "set hook error: {}", GetLastError());
        return false;
    }

    hooks.winMoveHook = SetWinEventHook(EVENT_SYSTEM_MOVESIZESTART,
                                        EVENT_SYSTEM_MOVESIZEEND,
                                        nullptr,
                                        winEventProc,
                                        0,
                                        0,
                                        WINEVENT_OUTOFCONTEXT |
                                            WINEVENT_SKIPOWNPROCESS);

    if (!hooks.winMoveHook) [[unlikely]] {
        fmt::println(stderr, "set win event hook error: {}", GetLastError());
        return false;
    }

    ctx.queue = queue;

    EventPause pause{};
    ctx.pauseToken = pause.token();
    queue->emplace(std::move(pause));

    isInit = true;

    return true;
}

void destory() noexcept {

    auto &ctx = Context::i();
    auto &hooks = Hooks::i();
    UnhookWindowsHookEx(hooks.kbHook);
    UnhookWinEvent(hooks.winMoveHook);

    if (ctx.pauseToken) {
        ctx.pauseToken->count_down();
        ctx.queue->emplace(EventQuit{});
    }
}

// NOLINTNEXTLINE(performance-unnecessary-value-param) 有什么话和标准库说去吧
void LLHook::run(std::stop_token token, EventQueue *const queue) noexcept {
    init(queue);
    MSG msg;
    while (!token.stop_requested()) [[unlikely]] {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) [[unlikely]] {
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
    destory();
}