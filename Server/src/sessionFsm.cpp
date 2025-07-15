#include "sessionFsm.h"

#include "messageBuilder.h"

namespace ses {

    void StateMachine::next(const Message& msg) {
        if (auto st = state_->transition(msg); st) {
            state_ = std::move(st);
        } else if (msg.type() != Exit) {
            throw InvalidTransitionException(
                fmt::format("{}: invalid transition to '{}'",
                            state_->getSession()->getData(),
                            magic_enum::enum_name(msg.type())));
        }
    }

    std::unique_ptr<IState> ConnectedState::transition(const Message& msg) {
        auto session = session_.lock();
        if (!session)
            throw NullSessionException("ConnectedState: session expired");

        switch (msg.type()) {
            case Register:
                fmt::println("{} <- Connected", session->getData());
                if (session->registerClient(msg.from_user())) {
                    return RegisteredState::create(session, fsm_);
                } else {
                    return ConnectedState::create(session, fsm_);
                }
                break;
            case Exit:
                fmt::println("{} <- Connected", session->getData());
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
                fmt::println("{} <- Registered", session->getData());
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
                fmt::println("{} <- Registered", session->getData());
                session->sendMessageToClient(
                    MessageBuilder::answerConfirmed(msg.from_user()));
                return AnsweringState::create(session, fsm_, msg.from_user());
                break;
            case Exit:
                fmt::println("{} <- Registered", session->getData());
                session->deleteClient(msg.from_user());
                session->close();
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
            case Accepted:  // from peer
                fmt::println("{} <- Calling", session->getData());
                session->sendMessageToClient(
                    MessageBuilder::talkConfirmed(msg.from_user(),
                                                  msg.to_user()));
                return TalkingState::create(session, fsm_, msg.to_user());
                break;
            case Rejected:  // from peer
                fmt::println("{} <- Calling", session->getData());
                session->sendMessageToClient(
                    MessageBuilder::talkDenied(msg.to_user()));
                return RegisteredState::create(session, fsm_);
                break;
            case End:  // from myself
                fmt::println("{} <- Calling", session->getData());
                session->sendMessageToSubscriberServer(
                    peer_,
                    MessageBuilder::rejectQuery());
                session->sendMessageToClient(MessageBuilder::talkDenied());
                return RegisteredState::create(session, fsm_);
                break;
            case Exit:  // from myself
                fmt::println("{} <- Calling", session->getData());
                session->sendMessageToSubscriberServer(
                    peer_,
                    MessageBuilder::rejectQuery());
                session->deleteClient(msg.from_user());
                session->close();
                return std::unique_ptr<IState>{};
                break;
            default:
                return std::unique_ptr<IState>{};
                break;
        }
    }

    std::unique_ptr<IState> AnsweringState::transition(const Message& msg) {
        auto session = session_.lock();
        if (!session)
            throw NullSessionException("AnsweringState: session expired");

        switch (msg.type()) {
            case Accept:  // from myself
                fmt::println("{} <- Answering", session->getData());
                session->getTimer()->cancel();
                session->sendMessageToSubscriberServer(
                    peer_,
                    MessageBuilder::talkConfirmed(peer_, msg.to_user()));
                session->sendMessageToClient(
                    MessageBuilder::talkConfirmed(peer_, msg.to_user()));
                return TalkingState::create(session, fsm_, peer_);
                break;
            case Reject:  // from myself OR peer
                fmt::println("{} <- Answering", session->getData());
                session->getTimer()->cancel();
                if (msg.from_user().empty()) {
                    // peer has rejected, without waiting for response
                    session->sendMessageToClient(MessageBuilder::talkDenied());
                } else {
                    // we have rejected, our name is in msg.from_user()
                    session->sendMessageToSubscriberServer(
                        peer_,
                        MessageBuilder::talkDenied(msg.from_user()));
                    session->sendMessageToClient(MessageBuilder::talkDenied());
                }
                return RegisteredState::create(session, fsm_);
                break;
            case Exit:  // from myself
                fmt::println("{} <- Answering", session->getData());
                session->getTimer()->cancel();
                session->sendMessageToSubscriberServer(
                    peer_,
                    MessageBuilder::talkDenied());
                session->deleteClient(msg.from_user());
                session->close();
                return std::unique_ptr<IState>{};
                break;
            default:
                return std::unique_ptr<IState>{};
                break;
        }
    }

    std::unique_ptr<IState> TalkingState::transition(const Message& msg) {
        auto session = session_.lock();
        if (!session)
            throw NullSessionException("TalkingState: session expired");

        switch (msg.type()) {
            case Text:
                fmt::println("{} <- Talking", session->getData());

                if (msg.from_user().empty()) {
                    // secondary packet - peer processes packet
                    session->sendMessageToClient(msg);
                } else {
                    // primary packet - send to peer
                    session->sendMessageToSubscriberServer(
                        peer_,
                        MessageBuilder::textQuery(msg.payload()));
                }
                return TalkingState::create(session, fsm_, peer_);
                break;
            case End:
                fmt::println("{} <- Talking", session->getData());

                if (msg.from_user().empty()) {
                    // secondary packet - peer processes packet
                    session->sendMessageToClient(MessageBuilder::talkEnded());
                } else {
                    // primary packet - send to peer
                    session->sendMessageToSubscriberServer(
                        peer_,
                        MessageBuilder::endQuery());
                    session->sendMessageToClient(MessageBuilder::talkEnded());
                }
                return RegisteredState::create(session, fsm_);
                break;
            case Exit:
                fmt::println("{} <- Talking", session->getData());
                session->sendMessageToSubscriberServer(
                    peer_,
                    MessageBuilder::endQuery());
                session->deleteClient(msg.from_user());
                session->close();
                return std::unique_ptr<IState>{};
                break;
            default:
                return std::unique_ptr<IState>{};
                break;
        }
    }

}  // namespace ses