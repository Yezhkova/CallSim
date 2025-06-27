#pragma once

#include "message.pb.h"
#include <optional>
#include <string>

class MessageBuilder {
   public:
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

   private:
    Message msg_;

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
    result += fmt::format("{} - ", buf);
    result += fmt::format("[{}] - ", msg.from_user());
    result += fmt::format("{}", msg.payload());

    return result;
}