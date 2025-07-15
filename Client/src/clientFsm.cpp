#include "clientFsm.h"

#include "messageBuilder.h"

namespace clt {
    void StateMachine::next(const Message& msg) {
        if (auto st = state_->transition(msg); st) {
            state_ = std::move(st);
        }
    }

    std::unique_ptr<IState> ConnectedState::transition(const Message& msg) {
        switch (msg.type()) {
            case Rejected:
                fmt::println("<- Connected");
                return ConnectedState::create(fsm_);
                break;
            case Registered:
                fmt::println("<- Connected");
                fsm_.onRegister(msg.to_user());
                return RegisteredState::create(fsm_);
                break;
            default:
                return std::unique_ptr<IState>{};
                break;
        }
    };

    std::unique_ptr<IState> RegisteredState::transition(const Message& msg) {
        switch (msg.type()) {
            case Calling:
                fmt::println("<- Registered");
                return CallingState::create(fsm_);
                break;
            case Answering:
                fmt::println("<- Registered");
                return AnsweringState::create(fsm_);
                break;
            default:
                return std::unique_ptr<IState>{};
                break;
        }
    };

    std::unique_ptr<IState> CallingState::transition(const Message& msg) {
        switch (msg.type()) {
            case Rejected:
                fmt::println("<- Calling");
                return RegisteredState::create(fsm_);
                break;
            case Accepted:
                fmt::println("<- Calling");
                return TalkingState::create(fsm_);
                break;
            default:
                return std::unique_ptr<IState>{};
                break;
        }
    };

    std::unique_ptr<IState> AnsweringState::transition(const Message& msg) {
        switch (msg.type()) {
            case Rejected:
                fmt::println("<- Answering");
                return RegisteredState::create(fsm_);
                break;
            case Accepted:
                fmt::println("<- Answering");
                return TalkingState::create(fsm_);
                break;
            default:
                return std::unique_ptr<IState>{};
                break;
        }
    };

    std::unique_ptr<IState> TalkingState::transition(const Message& msg) {
        switch (msg.type()) {
            case Text:
                fmt::println("<- Talking");
                return TalkingState::create(fsm_);
                break;
            case Ended:
                fmt::println("<- Talking");
                return RegisteredState::create(fsm_);
                break;
            default:
                return std::unique_ptr<IState>{};
                break;
        }
    }
}  // namespace clt