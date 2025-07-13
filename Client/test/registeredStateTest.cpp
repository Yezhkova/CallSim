#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "mockStateMachine.h"

using ::testing::_;
using ::testing::Return;
using ::testing::StrictMock;

class RegisteredClientStateTest : public ::testing::Test {
   protected:
    StrictMock<MockStateMachine> fsm;
    std::unique_ptr<clt::IState> state;

    void SetUp() override {
        state = std::make_unique<clt::RegisteredState>(fsm);
    }
};

TEST_F(RegisteredClientStateTest, CallingMessageTransitionsToCallingState) {
    Message msg  = createMessage(MessageType::Calling);
    auto    next = state->transition(msg);
    ASSERT_NE(next, nullptr);
    EXPECT_TRUE(dynamic_cast<clt::CallingState*>(next.get()) != nullptr);
}

TEST_F(RegisteredClientStateTest, AnsweringMessageTransitionsToAnsweringState) {
    Message msg  = createMessage(MessageType::Answering);
    auto    next = state->transition(msg);
    ASSERT_NE(next, nullptr);
    EXPECT_TRUE(dynamic_cast<clt::AnsweringState*>(next.get()) != nullptr);
}

TEST_F(RegisteredClientStateTest, UnknownMessageReturnsNullptr) {
    Message msg  = createMessage(MessageType::Text);
    auto    next = state->transition(msg);
    EXPECT_EQ(next, nullptr);
}
