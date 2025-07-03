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

    std::unique_ptr<IState> CallingState::transition(const Message& msg) {
        switch (msg.type()) {
            case Accepted:
                fmt::println("<- Calling");
                // TODO
                return TalkingState::create(fsm_);
                break;
            case Rejected:
                fmt::println("<- Calling");
                // TODO
                return RegisteredState::create(fsm_);
                break;
            default:
                return std::unique_ptr<IState>{};
                break;
        }
    };

    // timer_(fsm.client_.getContext())
    AnsweringState::AnsweringState(StateMachine&      fsm,
                                   const std::string& receiver)
      : IState(fsm), receiver_(receiver){};

    // void AnsweringState::enter() {
    //     timer_.expires_after(std::chrono::seconds(3));

    //     auto self = shared_from_this();
    //     timer_.async_wait([self](const boost::system::error_code& ec) {
    //         if (!ec) {
    //             self->onTimeout();
    //         }
    //     });
    // }

    void AnsweringState::onTimeout() {
        fsm_.client_.sendMessageToServer(MessageBuilder::callDenied(receiver_));
    }

    std::unique_ptr<IState> AnsweringState::transition(const Message& msg) {
        switch (msg.type()) {
                // case Rejected:
                //     fmt::println("<- Answering");
                //     break;

            default:
                return std::unique_ptr<IState>{};
                break;
        }
    };

    std::unique_ptr<IState> TalkingState::transition(const Message& msg) {
        switch (msg.type()) {
                // fmt::println("<- Calling");

            default:
                return std::unique_ptr<IState>{};
                break;
        }
    }
}  // namespace clt