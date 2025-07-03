#include "sessionFsm.h"

#include "messageBuilder.h"

namespace ses {

    void StateMachine::next(const Message& msg) {
        if (auto st = state_->transition(msg); st) {
            state_ = std::move(st);
        } else {
            throw InvalidTransitionException(
                fmt::format("{}: invalid transition to '{}'",
                            state_->getSession()->getEndpoint(),
                            magic_enum::enum_name(msg.type())));
        }
    }

    std::unique_ptr<IState> ConnectedState::transition(const Message& msg) {
        auto session = session_.lock();
        if (!session)
            throw NullSessionException("ConnectedState: session expired");

        switch (msg.type()) {
            case Register:
                fmt::println("{} <- Connected", session->getEndpoint());
                if (session->registerClient(msg.from_user())) {
                    return RegisteredState::create(session, fsm_);
                } else {
                    return ConnectedState::create(session, fsm_);
                }
                break;
            case Exit:
                session->close();
                return std::unique_ptr<IState>{};
            default:
                return std::unique_ptr<IState>{};
                break;
        }
    }

    std::unique_ptr<IState> RegisteredState::transition(const Message& msg) {
        auto session = session_.lock();
        if (!session)
            throw NullSessionException("RegisteredState: session expired");

        switch (msg.type()) {
            case Call:
                fmt::println("{} <- Registered", session->getEndpoint());
                if (session->callClient(msg.from_user(), msg.to_user())) {
                    session->sendMessageToClient(
                        MessageBuilder::callConfirmed(msg.to_user()));
                    return CallingState::create(session, fsm_);
                }
                session->sendMessageToClient(
                    MessageBuilder::callDenied(msg.to_user()));
                return RegisteredState::create(session, fsm_);
                break;
            case Answer:
                fmt::println("{} <- Registered", session->getEndpoint());
                session->sendMessageToClient(
                    MessageBuilder::answerConfirmed(msg.from_user()));
                return AnsweringState::create(session, fsm_);
                break;
            case Exit:
                session->deleteClient(msg.from_user());
                return std::unique_ptr<IState>{};
                break;
            default:
                return std::unique_ptr<IState>{};
                break;
        }
    }

    std::unique_ptr<IState> CallingState::transition(const Message& msg) {
        auto session = session_.lock();
        if (!session)
            throw NullSessionException("CallingState: session expired");

        switch (msg.type()) {
            case Exit:
                session->deleteClient(msg.from_user());
                return std::unique_ptr<IState>{};
                break;

            default:
                return std::unique_ptr<IState>{};
                break;
        }
    };

}  // namespace ses