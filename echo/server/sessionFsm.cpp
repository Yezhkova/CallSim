#include "sessionFsm.h"

#include "messageBuilder.h"

namespace ses {

    void StateMachine::next(const Message& msg) {
        auto st = state_->transition(msg);
        if (!st && msg.type() != Exit) {
            fmt::println("{}: invalid transition to '{}'",
                         state_->getSession()->getEndpoint(),
                         magic_enum::enum_name(msg.type()));
            state_->getSession()->sendMessage(
                MessageBuilder::operationDenied());
        } else if (st) {
            state_ = std::move(st);
        }
    }

    std::unique_ptr<IState> ConnectedState::transition(const Message& msg) {
        auto session = session_.lock();
        if (!session)
            return std::unique_ptr<IState>{};

        fmt::println("{} <- Connected", session->getEndpoint());

        if (msg.type() == Register) {
            if (session->registerClient(msg.from_user())) {
                return RegisteredState::create(session, fsm_);
            } else {
                return ConnectedState::create(session, fsm_);
            }
        }
        if (msg.type() == Exit) {
            session->deleteClient(msg.from_user());
            session->close();
            return std::unique_ptr<IState>{};
        }
        return std::unique_ptr<IState>{};
    }

    std::unique_ptr<IState> RegisteredState::transition(const Message& msg) {
        auto session = session_.lock();
        if (!session)
            return std::unique_ptr<IState>{};

        fmt::println("{} <- Registered", session->getEndpoint());

        if (msg.type() == Exit) {
            session->deleteClient(msg.from_user());
            session->close();
            return std::unique_ptr<IState>{};
        }
        return std::unique_ptr<IState>{};
    }

}  // namespace ses