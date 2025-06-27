#include "client.h"
#include "keyboard.h"
#include <boost/asio.hpp>

int main() {
    try {
        boost::asio::io_context io;
        auto                    endpoint =
            Tcp::endpoint(boost::asio::ip::make_address("127.0.0.1"), 12345);
        auto client = std::make_shared<Client>(io, endpoint);

        Keyboard kbd;

        kbd.onRegisterCommand = [client](const std::string& name) {
            client->onRegister(name);
        };

        kbd.onCallCommand = [client](const std::string& name) {
            fmt::print("{}", name);
        };

        kbd.onAcceptCommand = [client]() {};

        kbd.onRejectCommand = [client]() {};

        kbd.onTextCommand = [client](const std::string& name,
                                     const std::string& text) {
            fmt::print("{}{}", name, text);
        };

        kbd.onEndCommand = [client]() {};

        kbd.onExitCommand = [client]() { client->onExit(); };

        kbd.run();

        client->start();
        io.run();
        kbd.join();
    } catch (const std::exception& e) {
        fmt::println(stderr, "Client exception: {}", e.what());
    }
    return 0;
}
