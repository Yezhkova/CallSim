#pragma once

#include <fmt/core.h>

#include <iostream>

#include "boost/asio.hpp"
class Keyboard {
   private:
    std::thread thread_;

   public:
    using HandlerZeroArg   = std::function<void()>;
    using HandlerSingleArg = std::function<void(const std::string&)>;
    using HandlerDoubleArg =
        std::function<void(const std::string&, const std::string&)>;

    Keyboard() {}

    HandlerSingleArg onRegisterCommand;  // register <name>
    HandlerSingleArg onCallCommand;      // call <name>
    HandlerZeroArg   onAcceptCommand;    // accept
    HandlerZeroArg   onRejectCommand;    // reject
    HandlerDoubleArg onTextCommand;      // text <to_user> <message>
    HandlerZeroArg   onEndCommand;       // end <name>
    HandlerZeroArg   onExitCommand;      // exit

    void run();

    void join() {
        if (thread_.joinable())
            thread_.join();
    }
    static std::string trim_leading(const std::string& str);
};
