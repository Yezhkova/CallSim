#include "turnstile.h"
#include <fmt/core.h>

const Table transitions = {
    {{Locked, Coin},    {Unlocked, []{ fmt::print("Unlocked by coin\nCurrent state: Unlocked\n"); }}},
    {{Unlocked, Push},  {Locked,   []{ fmt::print("Pushed by person\nCurrent state: Locked\n"); }}}
};

State next(State src, Event evt) {
    auto it = transitions.find({src, evt});
    if (it == transitions.end()) {
        throw std::runtime_error("Invalid transition");
    }
    auto& [dst, fn] = it->second;
    fn();
    return dst;
};
