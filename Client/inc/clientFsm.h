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
        virtual void setLogin(const std::string& login)  = 0;
        virtual boost::asio::io_context& getContext()    = 0;
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
        ConnectedState(StateMachine& fsm) : IState(fsm){};
        std::unique_ptr<IState> transition(const Message& msg) override;

        static std::unique_ptr<IState> create(StateMachine& fsm) {
            fmt::println("-> Connected");
            return std::make_unique<ConnectedState>(fsm);
        };
    };

    struct RegisteredState : public IState {
        RegisteredState(StateMachine& fsm) : IState(fsm){};
        std::unique_ptr<IState> transition(const Message& msg) override;

        static std::unique_ptr<IState> create(StateMachine& fsm) {
            fmt::println("-> Registered");
            return std::make_unique<RegisteredState>(fsm);
        };
    };

    struct CallingState : public IState {
        CallingState(StateMachine& fsm) : IState(fsm){};
        std::unique_ptr<IState> transition(const Message& msg) override;

        static std::unique_ptr<IState> create(StateMachine& fsm) {
            fmt::println("-> Calling");
            return std::make_unique<CallingState>(fsm);
        };
    };

    struct AnsweringState
      : public IState,
        public std::enable_shared_from_this<AnsweringState> {
       private:
        std::string receiver_;
        // boost::asio::steady_timer timer_;

       public:
        AnsweringState(StateMachine& fsm, const std::string& receiver);
        std::unique_ptr<IState> transition(const Message& msg) override;

        void enter();

        static std::unique_ptr<IState> create(StateMachine&      fsm,
                                              const std::string& receiver) {
            fmt::println("-> Answering (with timeout)");
            auto answering = std::make_unique<AnsweringState>(fsm, receiver);
            // answering->enter();
            return answering;
        };

       private:
        void onTimeout();
    };

    struct TalkingState : public IState {
        TalkingState(StateMachine& fsm) : IState(fsm){};
        std::unique_ptr<IState> transition(const Message& msg) override;

        static std::unique_ptr<IState> create(StateMachine& fsm) {
            fmt::println("-> Talking");
            return std::make_unique<TalkingState>(fsm);
        };
    };

    struct StateMachine {
       public:
        IClientTransport&       client_;
        UiController&           ui_;
        std::unique_ptr<IState> state_;

        StateMachine(IClientTransport& client, UiController& ui);

        void next(const Message& msg);
    };

}  // namespace clt
