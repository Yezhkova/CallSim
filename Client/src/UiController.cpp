#include "UiController.h"

#include "messageBuilder.h"
#include <string>
namespace clt {

    void UiController::run() {
        thread_ = std::thread([this]() {
            std::string line;
            while (std::getline(std::cin, line)) {
                std::istringstream iss(line);
                std::string        command;
                iss >> command;

                std::transform(command.begin(),
                               command.end(),
                               command.begin(),
                               [](unsigned char c) { return std::tolower(c); });

                if (command == "exit") {
                    stopClient();
                    break;
                } else if (command == "register") {
                    std::string login;
                    iss >> login;
                    onMessageSend(MessageBuilder::registerQuery(login));
                } else if (command == "call") {
                } else if (command == "accept") {
                } else if (command == "reject") {
                } else if (command == "text") {
                    std::string target;
                    iss >> target;
                    std::string rest;
                    std::getline(iss, rest);
                } else if (command == "end") {
                } else {
                    fmt::println("[UiController] Unknown command: '{}'",
                                 command);
                }
            }
        });
    }

    void UiController::stopClient() {
        if (!username_.empty()) {
            onMessageSend(MessageBuilder::exitQuery(username_));
        }
        onCloseClientTransport();
    }

}  // namespace clt