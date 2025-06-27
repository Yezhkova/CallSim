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
    Session(Tcp::socket&& socket, std::weak_ptr<Server> server)
      : socket_(std::move(socket)), server_(std::move(server)), fsm_(*this){};

    void readHeader();
    void start() { readHeader(); };
    void readBody(std::shared_ptr<uint32_t> length);

    std::shared_ptr<Server> getServer() const { return server_.lock(); };
    void                    nextState(const Message& msg) { fsm_.next(msg); }

    void        sendMessage(const Message& msg) override;
    void        close() override { socket_.close(); }
    bool        registerClient(const std::string& name) override;
    bool        deleteClient(const std::string& name) override;
    std::string getEndpoint() override;
};