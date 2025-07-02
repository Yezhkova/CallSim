#include "uiController.h"
#include "client.h"
#include <boost/asio.hpp>

int main() {
    try {
        boost::asio::io_context io;
        auto                    endpoint =
            Tcp::endpoint(boost::asio::ip::make_address("127.0.0.1"), 12345);
        auto client_transport = std::make_shared<ClientTransport>(io, endpoint);

        clt::UiController ui;

        clt::StateMachine sm(*client_transport.get(), ui);

        client_transport->nextState = [&sm](const Message& msg) {
            sm.next(msg);
        };

        ui.onMessageSend = [&sm, client_transport](const Message& msg) {
            client_transport->sendMessageToServer(msg);
        };

        ui.onCloseClientTransport = [client_transport]() {
            client_transport->onExit();
        };

        boost::asio::signal_set signals(io, SIGINT, SIGTERM, SIGHUP);
        signals.async_wait([&](const boost::system::error_code&,
                               int signal_number) {
            fmt::println("Received signal {}, stopping client", signal_number);
            ui.stopClient();
        });

        ui.run();

        client_transport->start();
        io.run();
        ui.join();
        google::protobuf::ShutdownProtobufLibrary();
    } catch (const std::exception& e) {
        fmt::println(stderr, "ClientTransport exception: {}", e.what());
    }
    return 0;
}
