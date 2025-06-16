#include "turnstileOOP.h"

std::unique_ptr<IState> Locked::transition(Event evt) {
    if (evt == Event::Coin) {
        fmt::println("Unlocked by coin\nCurrent state: Unlocked");
        return Unlocked::create(fsm_);
    }
    return std::unique_ptr<IState>{};
}
std::unique_ptr<IState> Locked::create(StateMachine& fsm) {
    return std::make_unique<Locked>(fsm);
}

std::unique_ptr<IState> Unlocked::transition(Event evt) {
    if (evt == Event::Push) {
        fmt::println("Pushed by person\nCurrent state: Locked");
        return Locked::create(fsm_);
    }
    return std::unique_ptr<IState>{};
}
std::unique_ptr<IState> Unlocked::create(StateMachine& fsm) {
    return std::make_unique<Unlocked>(fsm);
}

void StateMachine::next(Event evt) {
    if (auto st = state_->transition(evt); st) {
        state_ = std::move(st);
        return;
    }
    throw std::runtime_error{"Invalid transition"};
}