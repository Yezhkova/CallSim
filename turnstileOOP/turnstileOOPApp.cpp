#include "turnstileOOP.h"
#include <iostream>
#include <magic_enum/magic_enum.hpp>

int main() {
    StateMachine fsm;
    std::string input;

    fmt::println("Current state: Locked");
    fmt::println("Enter 'Coin' or 'Push' (Ctrl+C to exit):");

    while (std::cin >> input) {
        auto evtOpt = magic_enum::enum_cast<Event>(input);
        try {
            fsm.next(*evtOpt);
        } catch (const std::exception& e) {
            fmt::println(stderr, "State Machine Error: {}", e.what());
        }
    }

    return 0;
}
