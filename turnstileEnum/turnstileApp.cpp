#include <iostream>
#include <magic_enum/magic_enum.hpp>

#include "turnstile.h"
// #include "turnstileSwitch.h"

int main() {
    State current = State::Locked;
    std::string input;

    fmt::println("Current state: {}", magic_enum::enum_name(current));
    fmt::println("Enter 'Coin' or 'Push'  (Ctrl+C to exit):");

    while (std::cin >> input) {
        auto evtOpt = magic_enum::enum_cast<Event>(input);
        try {
            current = next(current, evtOpt.value());
        } catch (const std::exception& ex) {
            fmt::println(stderr, "State Machine Error: {}", ex.what());
        }
    }
}
