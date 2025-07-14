#include "UiController.h"
#include "clientTransport.h"
#include <boost/asio.hpp>

int main() try {
    boost::asio::io_context io;

    auto endpoint =
        Tcp::endpoint(boost::asio::ip::make_address("127.0.0.1"), 12345);
    auto client_transport = std::make_shared<ClientTransport>(io, endpoint);

    clt::UiController ui(io);

    clt::StateMachine sm;
    sm.onRegister = [&client_transport, &ui](const std::string& login) {
        ui.setLogin(login);
    };

    client_transport->onMessageArrival = [&sm](const Message& msg) {
        sm.next(msg);
    };

    ui.onMessageSend = [&sm, client_transport](const Message& msg) {
        client_transport->sendMessageToServer(msg);
    };

    ui.onCloseClientTransport = [client_transport]() {
        client_transport->shutdown();
    };

    auto signals =
        std::make_shared<boost::asio::signal_set>(io, SIGINT, SIGTERM, SIGHUP);
    signals->async_wait([&ui, signals](const boost::system::error_code& ec,
                                       int signal_number) {
        if (!ec) {
            fmt::println("Received signal {}, stopping client", signal_number);
            ui.stopClient();
            ui.onCloseClientTransport();
            std::exit(0);
        } else {
            fmt::println(stderr, "Signal handler error: {}", ec.message());
        }
    });
    ui.run();
    client_transport->start();
    std::vector<std::thread> threads;
    for (int i = 0; i < 2; ++i) {
        threads.emplace_back([&io]() { io.run(); });
    }
    for (auto& th : threads) th.join();
    google::protobuf::ShutdownProtobufLibrary();

    return 0;
} catch (const std::exception& e) {
    fmt::println(stderr, "ClientTransport exception: {}", e.what());
    return 1;
}
