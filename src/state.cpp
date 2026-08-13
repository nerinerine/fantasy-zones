#include "state.hpp"

auto setState() noexcept -> State & {
    static State state{};
    return state;
}

auto getState() noexcept -> const State&{
    return setState();
}