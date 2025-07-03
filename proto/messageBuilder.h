#pragma once

#include <fmt/core.h>

#include "message.pb.h"
#include <magic_enum/magic_enum.hpp>
#include <optional>
#include <string>

class MessageBuilder {
   public:
    static Message registerQuery(const std::string& login) {
        return MessageBuilder()
            .type(Register)
            .from(login)
            .payload(fmt::format("'{}' is trying to register", login))
            .build();
    }
    static Message callQuery(const std::string& sender,
                             const std::string& receiver) {
        return MessageBuilder()
            .type(Call)
            .from(sender)
            .to(receiver)
            .payload(fmt::format("'{}' wants to call '{}'", sender, receiver))
            .build();
    }
    static Message answerQuery(const std::string& sender,
                               const std::string& receiver) {
        return MessageBuilder().type(Answer).from(sender).to(receiver).build();
    }
    static Message exitQuery(const std::string& name) {
        return MessageBuilder().type(Exit).from(name).build();
    }

    static Message registrationConfirmed(const std::string& login) {
        return MessageBuilder()
            .type(Registered)
            .to(login)
            .payload(fmt::format("'{}' registered successfully", login))
            .build();
    }
    static Message registrationDenied(const std::string& login) {
        std::string err =
            login.empty() ? "Your login cannot be empty."
                          : fmt::format("Login '{}' already exists.", login);
        return MessageBuilder()
            .type(Rejected)
            .to(login)
            .payload(err +
                     " Please enter another login: 'register <your_name>'")
            .build();
    }

    static Message callConfirmed(const std::string& receiver) {
        return MessageBuilder()
            .type(Calling)
            .payload(fmt::format("Calling '{}'", receiver))
            .build();
    }
    static Message callDenied(const std::string& receiver) {
        return MessageBuilder()
            .type(Rejected)
            .payload(
                fmt::format("The subscriber '{}' is unavailable", receiver))
            .build();
    }

    static Message answerConfirmed(const std::string& sender) {
        return MessageBuilder()
            .type(Answering)
            .payload(fmt::format(
                "'{}' is calling you. Please enter 'accept' or 'reject'",
                sender))
            .build();
    }
    static Message answerDenied(const std::string& receiver) {
        return MessageBuilder()
            .type(Rejected)
            .payload(fmt::format("The subscriber '{}' cannot answer your call",
                                 receiver))
            .build();
    }

    static Message operationDenied(MessageType type) {
        return MessageBuilder()
            .payload(fmt::format("Invalid transition to '{}'",
                                 magic_enum::enum_name(type)))
            .build();
    }

   private:
    Message msg_;

   private:
    MessageBuilder& type(MessageType type) {
        msg_.set_type(type);
        return *this;
    }

    MessageBuilder& from(const std::string& user) {
        msg_.set_from_user(user);
        return *this;
    }

    MessageBuilder& to(const std::string& user) {
        msg_.set_to_user(user);
        return *this;
    }

    MessageBuilder& payload(const std::string& data) {
        msg_.set_payload(data);
        return *this;
    }

    Message build() {
        msg_.set_timestamp(getCurrentTimestamp());
        return msg_;
    }

    static int64_t getCurrentTimestamp() {
        using namespace std::chrono;
        return duration_cast<milliseconds>(
                   system_clock::now().time_since_epoch())
            .count();
    }
};

inline std::string toPrintable(const Message& msg) {
    std::string result;
    std::time_t t = static_cast<std::time_t>(msg.timestamp() / 1000);
    char        buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&t));
    result += fmt::format("{}", buf);
    if (!msg.from_user().empty()) {
        result += fmt::format(" - [{}]", msg.from_user());
    }
    result += fmt::format(" - {}", msg.payload());

    return result;
}