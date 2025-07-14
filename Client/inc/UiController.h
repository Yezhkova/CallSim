#pragma once

#include <fmt/core.h>

#include <atomic>
#include <csignal>
#include <iostream>

#include "boost/asio.hpp"
struct Message;

namespace clt {
    class UiController {
       public:
        std::function<void(const Message& msg)> onMessageSend;
        std::function<void()>                   onCloseClientTransport;
        static std::atomic_bool                 shutdown_requested;

        static void handle_sigint(int) {
            shutdown_requested = true;
            // TODO: make exit here
        }

       private:
        boost::asio::io_context& io_;
        std::string              username_ = "";

       public:
        UiController(boost::asio::io_context& io) : io_(io) {}

        void run();

        void setLogin(const std::string& login) { username_ = login; }
        void stopClient();
    };
}  // namespace clt