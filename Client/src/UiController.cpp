#include "UiController.h"

#include "messageBuilder.h"
#include <string>
namespace clt {
    void UiController::run() {
        boost::asio::post(io_, [this]() {
            std::string line;
            while (std::getline(std::cin, line) && this->active_) {
                boost::asio::post(io_, [this, line]() {
                    std::istringstream iss(line);
                    std::string        command;
                    iss >> command;

                    std::transform(
                        command.begin(),
                        command.end(),
                        command.begin(),
                        [](unsigned char c) { return std::tolower(c); });

                    if (command == "exit") {
                        stopClient();
                        this->active_ = false;
                    } else if (command == "register") {
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
                        std::string messageText;
                        std::getline(iss, messageText);
                        onMessageSend(
                            MessageBuilder::textQuery(messageText, username_));
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

    void UiController::stopClient() {
        onMessageSend(MessageBuilder::exitQuery(username_));
        onCloseClientTransport();
    }

}  // namespace clt