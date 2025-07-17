#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "mockStateMachine.h"

using ::testing::_;
using ::testing::Return;
using ::testing::StrictMock;
using namespace clt;

class ConnectedClientStateTest : public ::testing::Test {
   protected:
    StrictMock<MockStateMachine> fsm;
    std::unique_ptr<IState>      state;

    void SetUp() override { state = std::make_unique<ConnectedState>(fsm); }
};

TEST_F(ConnectedClientStateTest, RejectedMessageReturnsConnectedState) {
    Message msg  = createMessage(MessageType::Rejected);
    auto    next = state->transition(msg);
    EXPECT_TRUE(next == nullptr);
}

TEST_F(ConnectedClientStateTest,
       RegisteredMessageCallsOnRegisterAndTransitions) {
    std::string expected_login = "alice";
    Message     msg = createMessage(MessageType::Registered, expected_login);
    EXPECT_CALL(fsm, onRegisterMock(expected_login)).Times(1);
    auto next = state->transition(msg);
    ASSERT_NE(next, nullptr);
    EXPECT_TRUE(dynamic_cast<RegisteredState*>(next.get()) != nullptr);
}

TEST_F(ConnectedClientStateTest, RejectedMessageKeepsStateConnected) {
    std::string attempted_login = "alice";
    Message     msg = createMessage(MessageType::Rejected, attempted_login);
    EXPECT_CALL(fsm, onRegisterMock).Times(0);
    auto next = state->transition(msg);
    EXPECT_TRUE(next == nullptr);
}

TEST_F(ConnectedClientStateTest, UnknownMessageTypeReturnsNullptr) {
    Message msg  = createMessage(MessageType::Call);
    auto    next = state->transition(msg);
    EXPECT_EQ(next, nullptr);
}

// TEST_F(ConnectedClientStateTest,
// RegisteredWithEmptyLoginCallsOnRegisterWithEmptyString) {
//     Message msg = createMessage(MessageType::Registered, "");
//     EXPECT_CALL(fsm, onRegisterMock("")).Times(1);

//     auto next = state->transition(msg);
//     ASSERT_NE(next, nullptr);
//     EXPECT_TRUE(dynamic_cast<RegisteredState*>(next.get()) != nullptr);
// }

TEST_F(ConnectedClientStateTest, UnknownMessageDoesNotCallOnRegister) {
    Message msg = createMessage(MessageType::Call);
    EXPECT_CALL(fsm, onRegisterMock).Times(0);

    auto next = state->transition(msg);
    EXPECT_EQ(next, nullptr);
}

TEST_F(ConnectedClientStateTest,
       ConsecutiveRejectedMessagesStayInConnectedState) {
    Message msg = createMessage(MessageType::Rejected);

    auto next1 = state->transition(msg);
    EXPECT_TRUE(next1 == nullptr);
    auto next2 = state->transition(msg); 
    EXPECT_TRUE(next2 == nullptr);  
}