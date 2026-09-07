#pragma once
#include <bit>
#include <memory>

// FPDelterFactory类，用来包装那些由sdl的工厂函数创建，并需要使用对应函数删除的指针。
// （如SDL_CreateWindow和SDL_DestroyWindow）使这些指针也接收RAII机制的管理
// 也可以用来管理SDL_Init和SDL_Quit这样的全局构造/析构函数
// ...也许不只能用在sdl上？

// 声明工厂模板
template <auto del> struct FPDelterFactory;
// 一个生成函数的简写
template <auto del> const auto MakeUniqueWith = FPDelterFactory<del>::gen;

// 正常deleter的特化
template <class T, void (*del)(T *)> struct FPDelterFactory<del> {
    //      ^^^ 萃取的目标类  ^^^deleter函数的指针

    // 使用gen函数调用
    static auto gen(T *ptr) {
        return std::unique_ptr<T, void (*)(T *)>(ptr, del);
    }

    // 实例化后直接作为函数调用
    // 就像这样：
    // auto i = SDLPtrFactory<Fn>{}();
    static auto operator()(T *ptr) {
        return gen(ptr);
    }
};

// 不接收参数的deleter的特化
template <void (*del)()> struct FPDelterFactory<del> {
    static auto gen() {
        // 一个随机选择的void类型非null指针
        // 仅作为std::unique_ptr持有的占位符使用
        // ** 不 要 解 引 用 它 !!! **
        const auto NOT_NULL_PTR = std::bit_cast<void *>(42ULL);
        const auto delWrapper = [](void *) -> void { del(); };
        using RetType = std::unique_ptr<void, decltype(delWrapper)>;

        return RetType(NOT_NULL_PTR, std::move(delWrapper));
    }

    static auto operator()() {
        return gen();
    }
};