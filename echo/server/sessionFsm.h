#pragma once

#include <fmt/base.h>

#include "message.pb.h"
#include <magic_enum/magic_enum.hpp>
#include <memory>
#include <stdexcept>

namespace ses {
    struct IState;
    struct StateMachine;

    class ISession {
       public:
        virtual bool        registerClient(const std::string& name) = 0;
        virtual bool        deleteClient(const std::string& name)   = 0;
        virtual void        sendMessage(const Message& msg)         = 0;
        virtual std::string getEndpoint()                           = 0;
        virtual void        close()                                 = 0;
    };

    struct IState {
       protected:
        ISession&     session_;
        StateMachine& fsm_;

       public:
        IState(ISession& session, StateMachine& fsm)
          : session_{session}, fsm_(fsm){};
        virtual ~IState()                                          = default;
        virtual std::unique_ptr<IState> transition(const Message&) = 0;
    };

    struct RegisteredState : IState {
        RegisteredState(ISession& session, StateMachine& fsm)
          : IState(session, fsm){};

        using IState::IState;
        std::unique_ptr<IState> transition(const Message& msg) override {
            // TODO
            fmt::println(stderr,
                         "RegisteredState: invalid transition to '{}'",
                         magic_enum::enum_name(msg.type()));

            return std::unique_ptr<IState>{};
        };

        static std::unique_ptr<IState> create(ISession&     session,
                                              StateMachine& fsm) {
            fmt::println("{} -> Registered", session.getEndpoint());
            return std::make_unique<RegisteredState>(session, fsm);
        };
    };

    struct ConnectedState : IState {
        ConnectedState(ISession& session, StateMachine& fsm)
          : IState(session, fsm){};

        using IState::IState;
        std::unique_ptr<IState> transition(const Message& msg) override {
            if (msg.type() == Register) {
                if (session_.registerClient(msg.from_user())) {
                    return RegisteredState::create(session_, fsm_);
                } else {
                    return ConnectedState::create(session_, fsm_);
                }
            }
            fmt::println(stderr,
                         "ConnectedState: invalid transition to '{}'",
                         magic_enum::enum_name(msg.type()));

            return std::unique_ptr<IState>{};
        }

        static std::unique_ptr<IState> create(ISession&     session,
                                              StateMachine& fsm) {
            fmt::println("{} -> Connected", session.getEndpoint());
            return std::make_unique<ConnectedState>(session, fsm);
        };
    };

    struct StateMachine {
       public:
        std::unique_ptr<IState> state_;
        StateMachine(ISession& session)
          : state_{ConnectedState::create(session, *this)} {};

        // void next(const Message& msg) {
        //     auto st = state_->transition(msg);
        //     if (!st && msg.type() != Exit) {
        //         throw std::runtime_error{"Invalid transition"};
        //     }
        //     if (st)
        //         state_ = std::move(st);
        // }

        void next(const Message& msg) {
            if (auto st = state_->transition(msg); st) {
                state_ = std::move(st);
                return;
            }
            throw std::runtime_error{"Invalid transition"};
        };
    };
}  // namespace ses