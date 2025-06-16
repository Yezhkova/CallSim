#include <boost/asio.hpp>
#include <fmt/core.h>

void run_server(boost::asio::io_context &io, unsigned short port);

int main()
{
    try
    {
        boost::asio::io_context io;
        run_server(io, 12345);
        io.run();
    }
    catch (const std::exception &e)
    {
        fmt::println(stderr, "Server exception: {}", e.what());
    }
    return 0;
}
