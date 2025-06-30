#include "clientFsm.h"

namespace clt {
    StateMachine::StateMachine(IClientTransport& client, UiController& ui)
      : client_(client), ui_(ui), state_{ConnectedState::create(*this)} {};

    void StateMachine::next(const Message& msg) {
        auto st = state_->transition(msg);
        if (!st && msg.type() != Exit) {
            fmt::println(stderr,
                         "Invalid transition to '{}'",
                         magic_enum::enum_name(msg.type()));
        } else if (st) {
            state_ = std::move(st);
        }
    }

    std::unique_ptr<IState> ConnectedState::transition(const Message& msg) {
        fmt::println("<- Connected");

        if (msg.type() == Register) {
            std::string err =
                msg.to_user().empty()
                    ? "Your login cannot be empty."
                    : fmt::format("Login '{}' already exists.", msg.to_user());
            fmt::println(
                "{} Please enter another login: 'register <your_name>'",
                err);
            return ConnectedState::create(fsm_);
        }
        if (msg.type() == Registered) {
            fsm_.client_.setLogin(msg.to_user());
            fsm_.ui_.setLogin(msg.to_user());
            fmt::println("{} registered successfully", msg.to_user());
            return RegisteredState::create(fsm_);
        }
        if (msg.type() == Exit) {
            fsm_.client_.sendMessageToServer(msg);
            return std::unique_ptr<IState>{};
        }
        return std::unique_ptr<IState>{};
    };

    std::unique_ptr<IState> RegisteredState::transition(const Message& msg) {
        fmt::println("<- Registered");

        if (msg.type() == Exit) {
            fsm_.client_.sendMessageToServer(msg);
            return std::unique_ptr<IState>{};
        }
        return std::unique_ptr<IState>{};
    };
}  // namespace clt
