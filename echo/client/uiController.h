#pragma once

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
        std::thread thread_;
        std::string username_ = "";

       public:
        UiController() {}

        void run();

        void join() {
            if (thread_.joinable())
                thread_.join();
        }

        void setLogin(const std::string& login) { username_ = login; }
        void stopClient();
    };
}  // namespace clt
