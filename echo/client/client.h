
#pragma once

#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <thread>

using boost::asio::ip::tcp;

class Client : public std::enable_shared_from_this<Client>
{
public:
    Client(boost::asio::io_context &io, tcp::endpoint endpoint)
        : socket_(io), endpoint_(endpoint) {}

    void start();
    void do_read();
    void run_input_thread();
    // void read_header();
    // void read_body(std::size_t length);

private:
    tcp::socket socket_;
    tcp::endpoint endpoint_;
};
