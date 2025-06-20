#include <fmt/core.h>

#include "client.h"
#include <boost/asio.hpp>
#include <memory>

int main() {
    try {
        boost::asio::io_context io;

        auto endpoint =
            Tcp::endpoint(boost::asio::ip::make_address("127.0.0.1"), 12345);

        auto client = std::make_shared<Client>(io, endpoint);
        client->start();
        client->run_input_thread();
        io.run();
    } catch (const std::exception& e) {
        fmt::println(stderr, "Client exception: {}", e.what());
    }
    return 0;
}
