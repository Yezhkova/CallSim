#pragma once

#include "sessionFsm.h"
#include <gmock/gmock.h>

class MockSession : public ses::ISession {
public:
    MOCK_METHOD(bool, registerClient, (const std::string&), (override));
    MOCK_METHOD(bool, deleteClient, (const std::string&), (override));
    MOCK_METHOD(bool, callClient, (const std::string&, const std::string&), (override));
    MOCK_METHOD(void, sendMessageToClient, (const Message&), (override));
    MOCK_METHOD(void, sendMessageToSubscriberServer, (const std::string&, const Message&), (override));
    MOCK_METHOD(void, sendMessageToSubscriberClient, (const std::string&, const Message&), (override));
    MOCK_METHOD(std::string, getEndpoint, (), (const, override));
    MOCK_METHOD(boost::asio::io_context&, getContext, (), (const, override));
    MOCK_METHOD(std::shared_ptr<boost::asio::steady_timer>, getTimer, (), (const, override));
    MOCK_METHOD(void, close, (), (override));
};
