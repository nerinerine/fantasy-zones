#pragma once
#include <latch>
#include <variant>
#include <fmt/format.h>

#include <etl/queue_spsc_atomic.h>

class EventQuit {
  public:
    static auto shouldQuit() noexcept -> bool & {
        static bool flag{ false };
        return flag;
    }
    void operator()() noexcept {
        shouldQuit() = true;
    }
};

class EventPause {
  public:
    EventPause() : pLatch(nullptr, cleanUp) {
        latchList().emplace(1);
        pLatch.reset(&(latchList().front()));
    }

    void operator()() noexcept {
        if (pLatch) {
            pLatch->wait();
        }
    }

    auto token() noexcept -> std::latch * {
        return pLatch.get();
    }

  private:
    static constexpr size_t LATCH_LIST_MAX_SIZE = 32;
    // 按理说这里使用一个的对象池会更符合语义，
    // 但考虑到EventPause只会在队列中先进先出，这意味着先创建的总是会先析构
    // 因此使用queue完全萌大奶~
    using LatchList = etl::queue_spsc_atomic<std::latch, LATCH_LIST_MAX_SIZE>;

    static auto latchList() noexcept -> LatchList & {
        static LatchList list{};
        return list;
    };

    static void cleanUp(std::latch *) noexcept {
        latchList().pop();
    }

    std::unique_ptr<std::latch, void (*)(std::latch *)> pLatch;
};

using Event = std::variant<EventPause, EventQuit>;

struct EventQueue {
    void process() noexcept {
        while (!queue.empty()) {
            auto &e = queue.front();
            std::visit([](auto &e) -> void { e(); }, e);
            queue.pop();
        }
    }

    static constexpr size_t EVENT_QUEUE_MAX_SIZE = 64;
    etl::queue_spsc_atomic<Event, EVENT_QUEUE_MAX_SIZE> queue;
};