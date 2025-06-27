#pragma once

#include <fmt/base.h>

#include "message.pb.h"
#include <magic_enum/magic_enum.hpp>
#include <memory>
#include <stdexcept>

namespace clt {
    struct IState;
    struct StateMachine;

    class IClient {
       public:
        virtual void sendMessage(const Message&)        = 0;
        virtual void setLogin(const std::string& login) = 0;
    };

    struct IState {
       protected:
        IClient&      client_;
        StateMachine& fsm_;

       public:
        IState(IClient& client, StateMachine& fsm)
          : client_{client}, fsm_(fsm) {}
        virtual ~IState()                                          = default;
        virtual std::unique_ptr<IState> transition(const Message&) = 0;
    };

    struct RegisteredState : IState {
        RegisteredState(IClient& client, StateMachine& fsm)
          : IState(client, fsm){};

        std::unique_ptr<IState> transition(const Message& msg) override {
            // TODO
            fmt::println(stderr,
                         "RegisteredState: invalid transition to '{}'",
                         magic_enum::enum_name(msg.type()));

            return std::unique_ptr<IState>{};
        };

        static std::unique_ptr<IState> create(IClient&      client,
                                              StateMachine& fsm) {
            fmt::println("-> Registered");
            return std::make_unique<RegisteredState>(client, fsm);
        };
    };

    struct ConnectedState : IState {
        ConnectedState(IClient& client, StateMachine& fsm)
          : IState(client, fsm){};

        std::unique_ptr<IState> transition(const Message& msg) override {
            if (msg.type() == Register) {
                client_.sendMessage(msg);
                return ConnectedState::create(client_, fsm_);
            }
            if (msg.type() == Rejected) {
                return ConnectedState::create(client_, fsm_);
            }
            if (msg.type() == Registered) {
                client_.setLogin(msg.to_user());
                return RegisteredState::create(client_, fsm_);
            }
            fmt::println(stderr,
                         "ConnectedState: invalid transition to '{}'",
                         magic_enum::enum_name(msg.type()));
            return std::unique_ptr<IState>{};
        };

        static std::unique_ptr<IState> create(IClient&      client,
                                              StateMachine& fsm) {
            fmt::println("-> Connected");
            return std::make_unique<ConnectedState>(client, fsm);
        };
    };

    struct StateMachine {
       public:
        std::unique_ptr<IState> state_;

        StateMachine(IClient& client)
          : state_{ConnectedState::create(client, *this)} {};

        void next(const Message& msg) {
            if (auto st = state_->transition(msg); st != nullptr) {
                state_ = std::move(st);
                return;
            }
            throw std::runtime_error{
                "[clt::StateMachine::next] Invalid transition"};
        };
    };
}  // namespace clt
