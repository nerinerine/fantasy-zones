#include "state.hpp"

auto getState() noexcept -> State & {
    static State state{};
    return state;
}