#include <boost/asio.hpp>
#include <iostream>

// Объявление функции из server.cpp
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
        std::cerr << "Server exception: " << e.what() << std::endl;
    }
    return 0;
}
