#include "clientFsm.h"

#include "messageBuilder.h"

namespace clt {
    StateMachine::StateMachine(IClientTransport& client, UiController& ui)
      : client_(client), ui_(ui), state_{ConnectedState::create(*this)} {};

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
                fsm_.client_.setLogin(msg.to_user());
                fsm_.ui_.setLogin(msg.to_user());
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
                return AnsweringState::create(fsm_, msg.to_user());
                break;
            default:
                return std::unique_ptr<IState>{};
                break;
        }
    };

}  // namespace clt