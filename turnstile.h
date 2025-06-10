#pragma once
#include <map>
#include <functional>
#include <optional>
#include <string>
#include <iostream>
#include <magic_enum.hpp>
#include <fmt/format.h>

enum State { Locked, Unlocked };
enum Event { Coin, Push };

template <>
struct fmt::formatter<State> : fmt::formatter<std::string_view> {
    auto format(State s, fmt::format_context& ctx) const {
        return fmt::formatter<std::string_view>::format(magic_enum::enum_name(s), ctx);
    }
};

inline std::ostream& operator<<(std::ostream& os, State s) {
    return os << magic_enum::enum_name(s);
}

using Table = std::map<std::pair<State, Event>, std::pair<State, std::function<void()>>>;

extern const Table transitions;
State next(State src, Event evt);
