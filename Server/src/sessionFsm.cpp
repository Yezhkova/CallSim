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
                break;
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
                    return CallingState::create(session, fsm_, msg.to_user());
                }
                session->sendMessageToClient(
                    MessageBuilder::callDenied(msg.to_user()));
                return RegisteredState::create(session, fsm_);
                break;
            case Answer:
                fmt::println("{} <- Registered", session->getEndpoint());
                session->sendMessageToClient(
                    MessageBuilder::answerConfirmed(msg.from_user()));
                return AnsweringState::create(session, fsm_, msg.from_user());
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
            case Accepted:
                fmt::println("{} <- Calling", session->getEndpoint());
                session->sendMessageToClient(
                    MessageBuilder::talkConfirmed(msg.from_user(),
                                                  msg.to_user()));
                return TalkingState::create(session, fsm_, msg.to_user());
                break;
            case Rejected:
                fmt::println("{} <- Calling", session->getEndpoint());
                session->sendMessageToClient(
                    MessageBuilder::talkDenied(msg.to_user()));
                return RegisteredState::create(session, fsm_);
                break;
            case End:
                fmt::println("{} <- Calling", session->getEndpoint());
                session->getTimer(peer_)->cancel();
                session->sendMessageToSubscriberServer(
                    peer_,
                    MessageBuilder::registerQuery(peer_));
                session->sendMessageToClient(
                    MessageBuilder::registrationConfirmed());
                return RegisteredState::create(session, fsm_);
                break;
            case Exit:
                session->getTimer(peer_)->cancel();
                session->sendMessageToSubscriberServer(
                    peer_,
                    MessageBuilder::registerQuery(peer_));
                session->deleteClient(msg.from_user());
                return std::unique_ptr<IState>{};
                break;
            default:
                return std::unique_ptr<IState>{};
                break;
        }
    };

std::unique_ptr<IState> AnsweringState::transition(const Message& msg) {
        auto session = session_.lock();
        if (!session)
            throw NullSessionException("AnsweringState: session expired");

        switch (msg.type()) {
            case Accept:
                fmt::println("{} <- Answering", session->getEndpoint());
                session->getTimer()->cancel();
                session->sendMessageToSubscriberServer(
                    peer_,
                    MessageBuilder::talkConfirmed(peer_, msg.to_user()));
                session->sendMessageToClient(
                    MessageBuilder::talkConfirmed(peer_, msg.to_user()));
                return TalkingState::create(session, fsm_, peer_);
                break;
            case Reject:
                fmt::println("{} <- Answering", session->getEndpoint());
                session->getTimer()->cancel();
                session->sendMessageToSubscriberServer(
                    peer_,
                    MessageBuilder::talkDenied(msg.to_user()));
                session->sendMessageToClient(MessageBuilder::talkDenied());
                return RegisteredState::create(session, fsm_);
                break;
            case Register:
                fmt::println("{} <- Answering", session->getEndpoint());
                session->sendMessageToClient(
                    MessageBuilder::registrationConfirmed());
                return RegisteredState::create(session, fsm_);
                break;
            case Exit:
                session->getTimer()->cancel();
                session->sendMessageToSubscriberServer(
                    peer_,
                    MessageBuilder::talkDenied(msg.to_user()));
                session->deleteClient(msg.from_user());
                return std::unique_ptr<IState>{};
                break;
            default:
                return std::unique_ptr<IState>{};
                break;
        }
    };

    std::unique_ptr<IState> TalkingState::transition(const Message& msg) {
        auto session = session_.lock();
        if (!session)
            throw NullSessionException("TalkingState: session expired");

        switch (msg.type()) {
            case Text:
                fmt::print("\r\033[K");
                fmt::println("{} <- Talking", session->getEndpoint());

                // session->sendMessageToSubscriberServer(
                //     peer_,
                //     MessageBuilder::talkConfirmed(msg.from_user(), peer_));
                session->sendMessageToSubscriberClient(peer_, msg);
                return TalkingState::create(session, fsm_, peer_);
                break;
            case End:
                fmt::println("{} <- Talking", session->getEndpoint());
                session->sendMessageToSubscriberServer(
                    peer_,
                    MessageBuilder::registerQuery(peer_));
                session->sendMessageToClient(
                    MessageBuilder::registrationConfirmed());
                return RegisteredState::create(session, fsm_);
                break;
            case Register:
                session->sendMessageToClient(
                    MessageBuilder::registrationConfirmed());
                return RegisteredState::create(session, fsm_);
                break;
            case Exit:
                session->sendMessageToSubscriberServer(
                    peer_,
                    MessageBuilder::registerQuery(peer_));
                session->deleteClient(msg.from_user());
                return std::unique_ptr<IState>{};
                break;
            default:
                return std::unique_ptr<IState>{};
                break;
        }
    };

}  // namespace ses