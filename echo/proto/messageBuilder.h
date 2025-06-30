#pragma once

#include "message.pb.h"
#include <optional>
#include <string>

class MessageBuilder {
   public:
    static Message registerQuery(const std::string& login) {
        return MessageBuilder().type(Register).from(login).build();
    }

    static Message exitQuery(const std::string& login) {
        return MessageBuilder().type(Exit).from(login).build();
    }

    static Message registrationConfirmed(const std::string& login) {
        return MessageBuilder().type(Registered).to(login).build();
    }
    static Message registrationDenied(const std::string& login) {
        return MessageBuilder().type(Register).to(login).build();
    }

    static Message operationDenied() { return MessageBuilder().build(); }

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
