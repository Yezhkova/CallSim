#include "UiController.h"

#include "messageBuilder.h"
#include <string>
namespace clt {
    void UiController::run() {
        boost::asio::post(io_, [this]() {
            std::string line;
            while (active_ && std::getline(std::cin, line)) {

                if (line.find("exit") != std::string::npos) {
                    onCloseClientTransport();
                    break;
                }
                boost::asio::post(io_, [this, line]() {
                    std::istringstream iss(line);
                    std::string        command;
                    iss >> command;
                    if (command == "register") {
                        std::string login;
                        iss >> login;
                        onMessageSend(MessageBuilder::registerQuery(login));
                    } else if (command == "call") {
                        std::string receiver;
                        iss >> receiver;
                        onMessageSend(
                            MessageBuilder::callQuery(username_, receiver));
                    } else if (command == "accept") {
                        onMessageSend(MessageBuilder::acceptQuery(username_));
                    } else if (command == "reject") {
                        onMessageSend(MessageBuilder::rejectQuery(username_));
                    } else if (command == "text") {
                        std::string message_text;
                        std::getline(iss, message_text);
                        onMessageSend(
                            MessageBuilder::textQuery(message_text, username_));
                    } else if (command == "end") {
                        onMessageSend(MessageBuilder::endQuery(username_));
                    } else {
                        fmt::println("[UiController] Unknown command: '{}'",
                                     command);
                    }
                });
            }
        });
    }

}  // namespace clt