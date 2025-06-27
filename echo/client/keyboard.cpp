#include "keyboard.h"

#include <string>
void Keyboard::run() {
    thread_ = std::thread([this]() {
        std::string line;
        while (std::getline(std::cin, line)) {
            std::istringstream iss(line);
            std::string        command;
            iss >> command;
            if (command == "exit") {
                onExitCommand();
            } else if (command == "register") {
                std::string name;
                iss >> name;
                onRegisterCommand(name);
            } else if (command == "call") {
                std::string target;
                iss >> target;
                onCallCommand(target);
            } else if (command == "accept") {
                onAcceptCommand();
            } else if (command == "reject") {
                onRejectCommand();
            } else if (command == "text") {
                std::string target;
                iss >> target;
                std::string rest;
                std::getline(iss, rest);
                onTextCommand(target, trim_leading(rest));
            } else if (command == "end") {
                // std::string target;
                // iss >> target;
                onEndCommand();
            } else {
                fmt::println("[Keyboard] Unknown command: '{}'", command);
            }
        }
    });
}

std::string Keyboard::trim_leading(const std::string& str) {
    size_t start = str.find_first_not_of(" \t");
    return (start == std::string::npos) ? "" : str.substr(start);
}
