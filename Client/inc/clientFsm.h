#pragma once

#include "UiController.h"
#include "message.pb.h"
#include <boost/asio/steady_timer.hpp>
#include <magic_enum/magic_enum.hpp>
#include <memory>
#include <stdexcept>

namespace clt {
    struct IState;
    struct StateMachine;

    class IClientTransport {
       public:
        virtual void sendMessageToServer(const Message&) = 0;
    };

    struct IState {
       protected:
        StateMachine& fsm_;

       public:
        IState(StateMachine& fsm) : fsm_(fsm) {}
        virtual ~IState()                                          = default;
        virtual std::unique_ptr<IState> transition(const Message&) = 0;
    };

    struct ConnectedState : public IState {
        ConnectedState(StateMachine& fsm) : IState(fsm) {}
        std::unique_ptr<IState> transition(const Message& msg) override;

        static std::unique_ptr<IState> create(StateMachine& fsm) {
            fmt::print(fg(fmt::color::blue), "-> Connected\n");
            return std::make_unique<ConnectedState>(fsm);
        }
        // ~ConnectedState() {
        //     fmt::print(fg(fmt::color::blue), "<- Connected\n");
        // }
    };

    struct RegisteredState : public IState {
        RegisteredState(StateMachine& fsm) : IState(fsm) {}
        std::unique_ptr<IState> transition(const Message& msg) override;

        static std::unique_ptr<IState> create(StateMachine& fsm) {
            fmt::print(fg(fmt::color::green), "-> Registered\n");
            return std::make_unique<RegisteredState>(fsm);
        }
        // ~RegisteredState() {
        //     fmt::print(fg(fmt::color::green), "<- Registered\n");
        // }
    };

    struct CallingState : public IState {
        CallingState(StateMachine& fsm) : IState(fsm) {}
        std::unique_ptr<IState> transition(const Message& msg) override;

        static std::unique_ptr<IState> create(StateMachine& fsm) {
            fmt::print(fg(fmt::color::magenta), "-> Calling\n");
            return std::make_unique<CallingState>(fsm);
        }
        // ~CallingState() { fmt::print(fg(fmt::color::magenta), "<- Calling\n"); }
    };

    struct AnsweringState : public IState {
        AnsweringState(StateMachine& fsm) : IState(fsm) {}
        std::unique_ptr<IState> transition(const Message& msg) override;

        static std::unique_ptr<IState> create(StateMachine& fsm) {
            fmt::print(fg(fmt::color::purple), "-> Answering (with timeout)\n");
            return std::make_unique<AnsweringState>(fsm);
        }
        // ~AnsweringState() {
        //     fmt::print(fg(fmt::color::purple), "<- Answering\n");
        // }
    };

    struct TalkingState : public IState {
        TalkingState(StateMachine& fsm) : IState(fsm) {}
        std::unique_ptr<IState> transition(const Message& msg) override;

        static std::unique_ptr<IState> create(StateMachine& fsm) {
            fmt::print(fg(fmt::color::chocolate), "-> Talking\n");
            return std::make_unique<TalkingState>(fsm);
        }
        // ~TalkingState() {
        //     fmt::print(fg(fmt::color::chocolate), "<- Talking\n");
        // }
    };

    struct StateMachine {
       public:
        std::unique_ptr<IState> state_;

        StateMachine() : state_{ConnectedState::create(*this)} {}

        std::function<void(const std::string& login)> onRegistered;
        void                                          next(const Message& msg);
    };

}  // namespace clt
