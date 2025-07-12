#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "mockStateMachine.h"

using ::testing::_;
using ::testing::Return;
using ::testing::StrictMock;

class ConnectedClientStateTest : public ::testing::Test {
   protected:
    StrictMock<MockStateMachine> fsm;
    std::unique_ptr<clt::IState> state;

    void SetUp() override {
        state = std::make_unique<clt::ConnectedState>(fsm);
    }
};

TEST_F(ConnectedClientStateTest, RejectedMessageReturnsConnectedState) {
    Message msg  = createMessage(MessageType::Rejected);
    auto    next = state->transition(msg);
    ASSERT_NE(next, nullptr);
    EXPECT_TRUE(dynamic_cast<clt::ConnectedState*>(next.get()));
}

TEST_F(ConnectedClientStateTest, RegisteredMessageCallsOnRegisterAndTransitions) {
    std::string expectedLogin = "alice";
    Message     msg = createMessage(MessageType::Registered, expectedLogin);
    EXPECT_CALL(fsm, onRegisterMock(expectedLogin)).Times(1);
    auto next = state->transition(msg);
    ASSERT_NE(next, nullptr);
    EXPECT_TRUE(dynamic_cast<clt::RegisteredState*>(next.get()) != nullptr);
}

TEST_F(ConnectedClientStateTest, RejectedMessageKeepsStateConnected) {
    std::string attemptedLogin = "alice";
    Message     msg = createMessage(MessageType::Rejected, attemptedLogin);
    EXPECT_CALL(fsm, onRegisterMock).Times(0);
    auto next = state->transition(msg);
    ASSERT_NE(next, nullptr);
    EXPECT_TRUE(dynamic_cast<clt::ConnectedState*>(next.get()) != nullptr);
}

TEST_F(ConnectedClientStateTest, UnknownMessageTypeReturnsNullptr) {
    Message msg  = createMessage(MessageType::Call);
    auto    next = state->transition(msg);
    EXPECT_EQ(next, nullptr);
}
