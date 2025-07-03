#pragma once
#include <fmt/base.h>

#include <stdexcept>

enum class State {
    Locked,
    Unlocked
};
enum class Event {
    Coin,
    Push
};

State next(State src, Event evt) {
    switch (src) {
        case State::Locked:
            if (evt == Event::Coin) {
                fmt::println("Unlocked by coin\nCurrent state: Unlocked");
                return State::Unlocked;
            }
            break;
        case State::Unlocked:
            if (evt == Event::Push) {
                fmt::println("Pushed by person\nCurrent state: Locked");
                return State::Locked;
            }
            break;
    }
    throw std::runtime_error{"Invalid transition"};
}