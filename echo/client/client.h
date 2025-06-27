#pragma once

#include <fmt/core.h>

#include "clientFsm.h"
#include "keyboard.h"
#include "message.pb.h"
#include <boost/asio.hpp>
#include <iostream>

using Tcp = boost::asio::ip::tcp;

class Client : public clt::IClient,
               public std::enable_shared_from_this<Client> {
   private:
    boost::asio::io_context& io_context_;
    Tcp::socket              socket_;
    Tcp::endpoint            endpoint_;
    clt::StateMachine        fsm_;
    std::string              login_ = "";

   public:
    Client(boost::asio::io_context& io, Tcp::endpoint endpoint)
      : io_context_(io), socket_(io), endpoint_(endpoint), fsm_(*this){};

    void start();
    void readHeader();
    void readBody(uint32_t length);

    void sendMessage(const Message& msg) override;

    void               onExit();
    void               onRegister(const std::string& name);
    void setLogin(const std::string& login) override { login_ = login; };

    void nextState(const Message& msg) { fsm_.next(msg); }
};
