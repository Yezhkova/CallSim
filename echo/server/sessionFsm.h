#pragma once

#include <fmt/base.h>

#include "message.pb.h"
#include <magic_enum/magic_enum.hpp>
#include <memory>
#include <stdexcept>

namespace ses {
    struct StateMachine;

    class ISession {
       public:
        virtual bool        registerClient(const std::string& name) = 0;
        virtual bool        deleteClient(const std::string& name)   = 0;
        virtual void        sendMessage(const Message& msg)         = 0;
        virtual std::string getEndpoint() const                     = 0;
        virtual void        close()                                 = 0;
    };

    struct IState {
       protected:
        std::weak_ptr<ISession> session_;
        StateMachine&           fsm_;

       public:
        IState(std::shared_ptr<ISession> session, StateMachine& fsm)
          : session_{session}, fsm_(fsm){};
        virtual ~IState()                                          = default;
        virtual std::unique_ptr<IState> transition(const Message&) = 0;

        std::shared_ptr<ISession> getSession() { return session_.lock(); }
    };

    struct ConnectedState : public IState {
        ConnectedState(std::shared_ptr<ISession> session, StateMachine& fsm)
          : IState(session, fsm){};

        std::unique_ptr<IState> transition(const Message& msg) override;

        static std::unique_ptr<IState> create(std::shared_ptr<ISession> session,
                                              StateMachine&             fsm) {
            fmt::println("{} -> Connected", session->getEndpoint());
            return std::make_unique<ConnectedState>(session, fsm);
        };
    };

    struct RegisteredState : public IState {
        RegisteredState(std::shared_ptr<ISession> session, StateMachine& fsm)
          : IState(session, fsm){};

        std::unique_ptr<IState> transition(const Message& msg) override;

        static std::unique_ptr<IState> create(std::shared_ptr<ISession> session,
                                              StateMachine&             fsm) {
            fmt::println("{} -> Registered", session->getEndpoint());
            return std::make_unique<RegisteredState>(session, fsm);
        };
    };

    struct StateMachine {
       public:
        std::unique_ptr<IState> state_ = nullptr;

        StateMachine(){};
        StateMachine(std::shared_ptr<ISession> session)
          : state_{ConnectedState::create(session, *this)} {}

        void next(const Message& msg);
    };
}  // namespace ses
