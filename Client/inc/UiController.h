#pragma once

#include <fmt/color.h>
#include <fmt/core.h>

#include <iostream>

#include "boost/asio.hpp"
struct Message;

namespace clt {
    class UiController {
       public:
        std::function<void(const Message& msg)> onMessageSend;
        std::function<void()>                   onCloseClientTransport;

       private:
        boost::asio::io_context& io_;
        std::string              username_ = "";

       public:
        UiController(boost::asio::io_context& io) : io_(io) {}
        std::atomic<bool> active_{true}; 

        void run();

        void setLogin(const std::string& login) { username_ = login; }
        const std::string& getLogin() { return username_; }
    };
}  // namespace clt