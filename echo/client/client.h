#pragma once

#include <fmt/core.h>

#include <boost/asio.hpp>
#include <iostream>

using Tcp = boost::asio::ip::tcp;

class Client : public std::enable_shared_from_this<Client> {
   public:
    Client(boost::asio::io_context& io, Tcp::endpoint endpoint)
      : socket_(io), endpoint_(endpoint) {}

    void start();
    void run_input_thread();
    void read_header();
    void read_body(uint32_t length);
    void send_message(const std::string& message);

   private:
    Tcp::socket   socket_;
    Tcp::endpoint endpoint_;
};
