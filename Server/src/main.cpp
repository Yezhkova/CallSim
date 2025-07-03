#include <fmt/core.h>

#include "server.h"
#include <boost/asio.hpp>

int main() {
    try {
        boost::asio::io_context io;
        auto                    server = std::make_shared<Server>(io, 12345);
        server->start();

        boost::asio::signal_set signals(io, SIGINT, SIGTERM, SIGHUP);
        signals.async_wait([&](const boost::system::error_code&,
                               int signal_number) {
            fmt::println(" Received signal {}, stopping server", signal_number);
            server->stop();
        });

        server->ioRun();
        google::protobuf::ShutdownProtobufLibrary();
    } catch (const std::exception& e) {
        fmt::println(stderr, "Server exception: {}", e.what());
    }
    return 0;
}