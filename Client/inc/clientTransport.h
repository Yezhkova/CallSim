#pragma once

#include <fmt/core.h>

#include "UiController.h"
#include "clientFsm.h"
#include "message.pb.h"
#include <boost/asio.hpp>
#include <functional>
#include <iostream>

using Tcp = boost::asio::ip::tcp;

class ClientTransport : public clt::IClientTransport,
                        public std::enable_shared_from_this<ClientTransport> {
   private:
    boost::asio::io_context& io_context_;
    Tcp::socket              socket_;
    Tcp::endpoint            endpoint_;
    std::string              login_ = "";

   public:
    ClientTransport(boost::asio::io_context& io, Tcp::endpoint endpoint)
      : io_context_(io), socket_(io), endpoint_(endpoint){};

    void start();
    void readHeader();
    void readBody(uint32_t length);

    void sendMessageToServer(const Message& msg) override;

    void onExit();
    void setLogin(const std::string& login) override { login_ = login; };
    boost::asio::io_context& getContext() override { return io_context_; };

    std::function<void(const Message& msg)> nextState;
};
