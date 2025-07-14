#pragma once

#include <gmock/gmock.h>

#include "../../proto/message.pb.h"
#include "../inc/clientFsm.h"

class MockStateMachine : public clt::StateMachine {
   public:
    MOCK_METHOD(void, onRegisterMock, (const std::string&), ());
    MockStateMachine() {
        onRegister = [this](const std::string& login) {
            onRegisterMock(login);
        };
    }
};

inline Message createMessage(MessageType        type,
                             const std::string& to_user = "") {
    Message msg;
    msg.set_type(type);
    msg.set_to_user(to_user);
    return msg;
}
