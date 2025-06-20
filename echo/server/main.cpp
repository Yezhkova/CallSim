#include <fmt/core.h>

#include <boost/asio.hpp>

void run_server(boost::asio::io_context& io, uint16_t port);

int main() {
    try {
        boost::asio::io_context io;
        run_server(io, 12345);
        io.run();
    } catch (const std::exception& e) {
        fmt::println(stderr, "Server exception: {}", e.what());
    }
    return 0;
}
