#pragma once
#include <fmt/base.h>
#include <functional>
#include <map>
#include <stdexcept>

enum class State { Locked, Unlocked };
enum class Event { Coin, Push };

using Table = std::map<std::pair<State, Event>, std::pair<State, std::function<void()>>>;

static const Table transitions = {
    {{State::Locked, Event::Coin},
        {State::Unlocked, [] { fmt::println("Unlocked by coin\nCurrent state: Unlocked"); }}},
    {{State::Unlocked, Event::Push},
        {State::Locked, [] { fmt::println("Pushed by person\nCurrent state: Locked"); }}}};

State next(State src, Event evt) {
    auto it = transitions.find({src, evt});
    if (it == transitions.end()) {
        throw std::runtime_error("Invalid transition");
    }
    auto& [dst, fn] = it->second;
    fn();
    return dst;
};