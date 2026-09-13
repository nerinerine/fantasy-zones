#include "hook.hpp"

#include <SDL3/SDL.h>
#include <fmt/base.h>
#include <etl/singleton.h>

#define USE_MACRO
#include "../helper/make_unique_with.hpp"
#undef USE_MACRO
#include "../helper/events.hpp"

#include <Windows.h>

// 几个仅应当作为单例模式创建的类
// 一般来说其实不推荐在多线程环境下使用单例模式
// 但是一方面来说，这几个类仅在单个线程内的少量函数中使用
// 另一方面来说，微软的win32api的callback
// 既不能使用lambda透传大法，也不提供userdata指针
// 单例甚至已经是其中最优雅的方案了
// 总之微软你会一直顺(

// anywway，因为这几个单例在创建之后就不再被销毁/移动
// 所以各个函数也只需使用static变量获取一次引用即可

struct UserState {
    bool isWindowMoving = false;
    bool isAltkeyPushing = false;
};

struct Hooks {
  public:
    HHOOK kbHook{};
    HWINEVENTHOOK winMoveHook{};
};

void submitWindowEvent() noexcept {
    static bool isActive = false;
    static const auto &kb = etl::singleton<UserState>::instance();
    static auto &events = etl::singleton<Events>::instance();

    if (kb.isWindowMoving && kb.isAltkeyPushing) [[unlikely]] {
        // 满足激活条件，且处在暂停状态
        SDL_PushEvent(&events.displayEvent);
        isActive = true;
    } else if (isActive && !kb.isWindowMoving) [[unlikely]] {
        // 处在激活状态时停止移动窗口
        SDL_PushEvent(&events.submitEvent);
        isActive = false;
    } else if (isActive && !kb.isAltkeyPushing) [[unlikely]] {
        // 处在激活状态时松开alt
        SDL_PushEvent(&events.cancelEvent);
        isActive = false;
    } //else 什么都不做
}

auto CALLBACK keyboardLLEventProc(int nCode,
                                  WPARAM wParam,
                                  LPARAM lParam) noexcept -> LRESULT {
    static auto &kb = etl::singleton<UserState>::instance();
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

    return CallNextHookEx(etl::singleton<Hooks>::instance().kbHook,
                          nCode,
                          wParam,
                          lParam);
}

// NOLINTBEGIN(bugprone-easily-swappable-parameters)
// win32api底层逻辑这一块...哦对了,
void CALLBACK winEventProc(HWINEVENTHOOK hWinEventHook,
                           DWORD event,
                           HWND hwnd,
                           LONG idObject,
                           LONG idChild,
                           DWORD idEventThread,
                           DWORD dwmsEventTime) noexcept {
    // NOLINTEND(bugprone-easily-swappable-parameters)
    static auto &kb = etl::singleton<UserState>::instance();
    if (event == EVENT_SYSTEM_MOVESIZESTART) {
        kb.isWindowMoving = true;
    }
    if (event == EVENT_SYSTEM_MOVESIZEEND) {
        kb.isWindowMoving = false;
    }
    submitWindowEvent();
}

// NOLINTNEXTLINE(performance-unnecessary-value-param) 有什么话和标准库说去吧
void llHookRun(std::stop_token token) noexcept {
    MSG msg;
    // 下面这段把win event接入stop_callback的操作是d指导的灵感
    // 其实一开始我的想法是把这个线程用win32api创建
    // 这样可以直接在这个线程的事件队列里拉屎
    // 但是d指导的方案显然更有美感
    // 大抵是从哪里摘抄的前人作品吧(目移)

    // 提前PeekMessage一次创建消息队列
    PeekMessage(&msg, nullptr, 0, 0, PM_NOREMOVE);
    // 创建一个可以被wait的handle
    HANDLE hStop = CreateEvent(nullptr, TRUE, FALSE, nullptr);

    if (!hStop) {
        fmt::println(stderr, "create win32 event error: {}", GetLastError());
        return;
    }

    auto hStopHandle = MakeUniqueWith<toFP<HANDLE &>(
        [](HANDLE &hStop) -> void { CloseHandle(hStop); })>(hStop);

    // 当这个线程被stop_request时发送事件
    // 这里的回调是由主线程执行的，所以也就不用担心阻塞
    std::stop_callback cb(token, [hStop] noexcept -> void { SetEvent(hStop); });

    MAKE_UNIQUE_FOR(HHOOK__)
    kbHook{ SetWindowsHookEx(WH_KEYBOARD_LL,      //hook 目标
                             keyboardLLEventProc, // 回调
                             nullptr,             //意义不明的参数
                             0                    //0表示全局hook
                             ),
            [](HHOOK hook) noexcept -> void { UnhookWindowsHookEx(hook); } };

    if (!kbHook) [[unlikely]] {
        fmt::println(stderr, "set hook error: {}", GetLastError());
        return;
    }

    MAKE_UNIQUE_FOR(HWINEVENTHOOK__)
    winMoveHook{ SetWinEventHook(EVENT_SYSTEM_MOVESIZESTART,
                                 EVENT_SYSTEM_MOVESIZEEND,
                                 nullptr,
                                 winEventProc,
                                 0,
                                 0,
                                 WINEVENT_OUTOFCONTEXT |
                                     WINEVENT_SKIPOWNPROCESS),
                 [](HWINEVENTHOOK hook) noexcept -> void {} };

    if (!winMoveHook) [[unlikely]] {
        fmt::println(stderr, "set hook error: {}", GetLastError());
        return;
    }

    etl::singleton<Hooks>::create(
        Hooks{ .kbHook = kbHook.get(), .winMoveHook = winMoveHook.get() });

    etl::singleton<UserState>::create();

    // while (!token.stop_requested()) [[unlikely]] {
    // 上面设置了回调，这里就不用反复检测token了
    while (true) {
        // 第一个入参是需要等待的handle的列表长度，第二个则是列表的本体（handle*）
        // 和包括win32api在内的大部分c api完全相反的使用习惯，有点神了
        const auto ret =
            MsgWaitForMultipleObjects(1, &hStop, FALSE, INFINITE, QS_ALLINPUT);

        // WAIT_OBJECT_0指列表中第一个需要等待的对象
        // 这里传入了一个handle那自然是没问题了
        // 你以为第二个是WAIT_OBJECT_1?
        // 没想到吧其实是WAIT_OBJECT_0 + 1
        // ...有点变态了
        if (ret == WAIT_OBJECT_0) [[unlikely]] {
            break; // 收到停止请求
        }

        if (ret == WAIT_OBJECT_0 + 1) [[likely]] {
            while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) [[unlikely]] {
                // 其实这个检测应该没什么用，但是D指导这么写了...(目移)
                if (msg.message == WM_QUIT)
                    break;

                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }
}