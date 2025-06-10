#include <fmt/core.h>
#include "turnstile.h"

int main() {
    State current = Locked;
    std::string input;

    fmt::print("Current state: {}\n", current);
    fmt::print("Enter 'Coin' or 'Push'  (Ctrl+C to exit):\n");

    while (std::cin >> input) {
        auto evtOpt = magic_enum::enum_cast<Event>(input);
        if (!evtOpt) {
            fmt::print("Input Error: event {} does not exist\n", input);
            fmt::print("Current state: {}\n", current);
            continue;
        }

        try {
            current = next(current, evtOpt.value());
        } catch (const std::exception& ex) {
            fmt::print(stderr, "State Machine Error: {}\n", ex.what());
        }
    }
}
