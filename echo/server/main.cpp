#include <fmt/core.h>

#include "server.h"
#include <boost/asio.hpp>

int main() {
    try {
        boost::asio::io_context io;
        auto server = std::make_shared<Server>(io, 12345);
        server->start();
        server->ioRun();
    } catch (const std::exception& e) {
        fmt::println(stderr, "Server exception: {}", e.what());
    }
    return 0;
}
