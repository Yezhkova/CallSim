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
                fmt::print(fg(fmt::color::blue), "<- Connected\n");
                return ConnectedState::create(fsm_);
                break;
            case Registered:
                fmt::print(fg(fmt::color::blue), "<- Connected\n");
                fsm_.onRegistered(msg.to_user());
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
                fmt::print(fg(fmt::color::green), "<- Registered\n");
                fmt::println("Enter 'end' to stop calling.");
                return CallingState::create(fsm_);
                break;
            case Answering:
                fmt::print(fg(fmt::color::green), "<- Registered\n");
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
                fmt::print(fg(fmt::color::magenta), "<- Calling\n");
                return RegisteredState::create(fsm_);
                break;
            case Accepted:
                fmt::print(fg(fmt::color::magenta), "<- Calling\n");
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
                fmt::print(fg(fmt::color::red), "<- Answering\n");
                return RegisteredState::create(fsm_);
                break;
            case Accepted:
                fmt::print(fg(fmt::color::red), "<- Answering\n");
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
                fmt::print(fg(fmt::color::chocolate), "<- Talking\n");
                return TalkingState::create(fsm_);
                break;
            case Ended:
                fmt::print(fg(fmt::color::chocolate), "<- Talking\n");
                return RegisteredState::create(fsm_);
                break;
            default:
                return std::unique_ptr<IState>{};
                break;
        }
    }
}  // namespace clt