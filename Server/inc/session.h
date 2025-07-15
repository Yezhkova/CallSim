#pragma once

#include <fmt/core.h>

#include "message.pb.h"
#include "messageBuilder.h"
#include "sessionFsm.h"
#include <boost/asio.hpp>

using Tcp = boost::asio::ip::tcp;

class Server;

class Session : public ses::ISession,
                public std::enable_shared_from_this<Session> {
   private:
    Tcp::socket           socket_;
    std::weak_ptr<Server> server_;
    std::vector<char>     body_buf_;
    ses::StateMachine     fsm_;

   public:
    std::shared_ptr<boost::asio::steady_timer> timer_;
    std::string                                username_;

   public:
    Session(Tcp::socket&& socket, std::weak_ptr<Server> server)
      : socket_(std::move(socket)), server_(std::move(server)) {}

    void readHeader();
    void start() {
        fsm_   = ses::StateMachine(shared_from_this());
        timer_ = std::make_shared<boost::asio::steady_timer>(getContext());
        readHeader();
    }
    void readBody(std::shared_ptr<uint32_t> length);

    std::shared_ptr<Server> getServer() const;
    void                    nextState(const Message& msg) { fsm_.next(msg); }

    void sendMessageToClient(const Message& msg) override;
    void sendMessageToSubscriberServer(const std::string& name,
                                       const Message&     msg) override;
    void sendMessageToSubscriberClient(const std::string& name,
                                       const Message&     msg) override;
    void close() override;
    bool registerClient(const std::string& name) override;
    bool callClient(const std::string& sender,
                    const std::string& receiver) override;
    bool deleteClient(const std::string& name) override;

    std::string                                getEndpoint() const override;
    boost::asio::io_context&                   getContext() const override;
    std::shared_ptr<boost::asio::steady_timer> getTimer() const override;

    bool               isOpen() { return socket_.is_open(); }
    ~Session();
};