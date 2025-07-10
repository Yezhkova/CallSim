#pragma once

#include <fmt/core.h>

#include "message.pb.h"
#include "messageBuilder.h"
#include <boost/asio.hpp>
#include <magic_enum/magic_enum.hpp>
#include <memory>
#include <stdexcept>

class NullSessionException : public std::runtime_error {
   public:
    explicit NullSessionException(const std::string& msg)
      : std::runtime_error(msg) {}
};

class InvalidTransitionException : public std::runtime_error {
   public:
    explicit InvalidTransitionException(const std::string& msg)
      : std::runtime_error(msg) {}
};

namespace ses {
    struct StateMachine;

    class ISession {
       public:
        virtual bool registerClient(const std::string& name)           = 0;
        virtual bool deleteClient(const std::string& name)             = 0;
        virtual bool callClient(const std::string& sender,
                                const std::string& receiver)           = 0;
        virtual void sendMessageToClient(const Message& msg)           = 0;
        virtual void sendMessageToSubscriberServer(const std::string& name,
                                                   const Message&     msg) = 0;
        virtual void sendMessageToSubscriberClient(const std::string& name,
                                                   const Message&     msg) = 0;

        virtual std::string              getEndpoint() const = 0;
        virtual boost::asio::io_context& getContext() const  = 0;
        virtual std::shared_ptr<boost::asio::steady_timer> getTimer(
            const std::string& name = "") const = 0;

        virtual void close() = 0;
    };

    struct IState {
       protected:
        std::weak_ptr<ISession> session_;
        StateMachine&           fsm_;

       public:
        IState(std::shared_ptr<ISession> session, StateMachine& fsm)
          : session_{session}, fsm_(fsm) {}
        virtual ~IState()                                          = default;
        virtual std::unique_ptr<IState> transition(const Message&) = 0;

        std::shared_ptr<ISession> getSession() { return session_.lock(); }
    };

    struct ConnectedState : public IState {
        ConnectedState(std::shared_ptr<ISession> session, StateMachine& fsm)
          : IState(session, fsm) {}

        std::unique_ptr<IState> transition(const Message& msg) override;

        static std::unique_ptr<IState> create(std::shared_ptr<ISession> session,
                                              StateMachine&             fsm) {
            fmt::println("{} -> Connected", session->getEndpoint());
            return std::make_unique<ConnectedState>(session, fsm);
        }
    };

    struct RegisteredState : public IState {
        RegisteredState(std::shared_ptr<ISession> session, StateMachine& fsm)
          : IState(session, fsm) {}

        std::unique_ptr<IState> transition(const Message& msg) override;

        static std::unique_ptr<IState> create(std::shared_ptr<ISession> session,
                                              StateMachine&             fsm) {
            fmt::println("{} -> Registered", session->getEndpoint());
            return std::make_unique<RegisteredState>(session, fsm);
        }
    };

    struct CallingState : public IState {
        std::string peer_;

        CallingState(std::shared_ptr<ISession> session,
                     StateMachine&             fsm,
                     const std::string&        peer)
          : IState(session, fsm), peer_(peer) {}

        std::unique_ptr<IState> transition(const Message& msg) override;

        static std::unique_ptr<IState> create(std::shared_ptr<ISession> session,
                                              StateMachine&             fsm,
                                              const std::string&        peer) {
            fmt::println("{} -> Calling", session->getEndpoint());
            return std::make_unique<CallingState>(session, fsm, peer);
        }
    };

    struct AnsweringState : public IState {
        std::string peer_;

        AnsweringState(std::shared_ptr<ISession> session,
                       StateMachine&             fsm,
                       const std::string&        peer)
          : IState(session, fsm), peer_(peer) {}

        std::unique_ptr<IState> transition(const Message& msg) override;

        static std::unique_ptr<IState> create(std::shared_ptr<ISession> session,
                                              StateMachine&             fsm,
                                              const std::string&        peer) {
            fmt::println("{} -> Answering", session->getEndpoint());
            return std::make_unique<AnsweringState>(session, fsm, peer);
        }
    };

    struct TalkingState : public IState {
        std::string peer_;

        TalkingState(std::shared_ptr<ISession> session,
                     StateMachine&             fsm,
                     const std::string&        peer)
          : IState(session, fsm), peer_(peer) {}

        std::unique_ptr<IState> transition(const Message& msg) override;

        static std::unique_ptr<IState> create(std::shared_ptr<ISession> session,
                                              StateMachine&             fsm,
                                              const std::string&        peer) {
            fmt::println("{} -> Talking", session->getEndpoint());
            return std::make_unique<TalkingState>(session, fsm, peer);
        }
    };

    struct StateMachine {
       public:
        std::unique_ptr<IState> state_ = nullptr;

        StateMachine() {}
        StateMachine(std::shared_ptr<ISession> session)
          : state_{ConnectedState::create(session, *this)} {}

        void next(const Message& msg);
    };
}  // namespace ses