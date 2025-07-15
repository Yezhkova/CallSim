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
    sm.onRegistered = [client_transport, &ui](const std::string& login) {
        ui.setLogin(login);
    };

    client_transport->onMessageArrival = [&sm](const Message& msg) {
        sm.next(msg);
    };

    ui.onMessageSend = [&sm, client_transport](const Message& msg) {
        client_transport->sendMessageToServer(msg);
    };

    ui.onCloseClientTransport = [&ui, client_transport]() {
        ui.active_ = false;
        client_transport->shutdown();
    };

    auto cleanup = std::unique_ptr<void, std::function<void(void*)>>{
        nullptr,
        [](void*) {
            fmt::println("Shutting down Protobuf...");
            google::protobuf::ShutdownProtobufLibrary();
        }};

    ui.run();
    client_transport->start();

    std::vector<std::thread> threads;
    for (int i = 0; i < 2; ++i) {
        threads.emplace_back([&io]() { io.run(); });
    }
    for (auto& th : threads) th.join();
    fmt::print("leaving main\n");
    return 0;
} catch (const std::exception& e) {
    fmt::println(stderr, "ClientTransport exception: {}", e.what());
    return 1;
}
