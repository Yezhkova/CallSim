#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "sessionFsm.h"
#include "mockSession.h"
#include "message.pb.h"

using ::testing::_;
using ::testing::Return;
using ::testing::StrictMock;

class ConnectedServerStateTest : public ::testing::Test {
protected:
    std::shared_ptr<StrictMock<MockSession>> session;
    ses::StateMachine fsm;
    std::unique_ptr<ses::IState> state;

    void SetUp() override {
        session = std::make_shared<StrictMock<MockSession>>();
        ON_CALL(*session, getEndpoint()).WillByDefault(Return("127.0.0.1:1111"));
        state = std::make_unique<ses::ConnectedState>(session, fsm);
    }

    Message makeMessage(MessageType type, const std::string& from = "") {
        Message msg;
        msg.set_type(type);
        msg.set_from_user(from);
        return msg;
    }
};

TEST_F(ConnectedServerStateTest, RegisterSuccessTransitionsToRegistered) {
    EXPECT_CALL(*session, registerClient("alice")).WillOnce(Return(true));
    EXPECT_CALL(*session, getEndpoint()).Times(2);

    Message msg = makeMessage(MessageType::Register, "alice");
    auto next = state->transition(msg);

    ASSERT_NE(next, nullptr);
    EXPECT_TRUE(dynamic_cast<ses::RegisteredState*>(next.get()));
}

TEST_F(ConnectedServerStateTest, RegisterFailsStaysInConnected) {
    EXPECT_CALL(*session, registerClient("bob")).WillOnce(Return(false));
    EXPECT_CALL(*session, getEndpoint()).Times(2);

    Message msg = makeMessage(MessageType::Register, "bob");
    auto next = state->transition(msg);

    ASSERT_NE(next, nullptr);
    EXPECT_TRUE(dynamic_cast<ses::ConnectedState*>(next.get()));
}

TEST_F(ConnectedServerStateTest, ExitClosesSessionAndReturnsNull) {
    EXPECT_CALL(*session, close()).Times(1);

    Message msg = makeMessage(MessageType::Exit);
    auto next = state->transition(msg);

    EXPECT_EQ(next, nullptr);
}

TEST_F(ConnectedServerStateTest, UnknownMessageReturnsNullptr) {
    Message msg = makeMessage(MessageType::Call);
    auto next = state->transition(msg);
    EXPECT_EQ(next, nullptr);
}
