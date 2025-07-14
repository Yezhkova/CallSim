#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "mockStateMachine.h"

using ::testing::_;
using ::testing::Return;
using ::testing::StrictMock;
using namespace clt;

class RegisteredClientStateTest : public ::testing::Test {
   protected:
    StrictMock<MockStateMachine> fsm;
    std::unique_ptr<::IState>    state;

    void SetUp() override { state = std::make_unique<RegisteredState>(fsm); }
};

TEST_F(RegisteredClientStateTest, CallingMessageTransitionsToCallingState) {
    Message msg  = createMessage(MessageType::Calling);
    auto    next = state->transition(msg);
    ASSERT_NE(next, nullptr);
    EXPECT_TRUE(dynamic_cast<CallingState*>(next.get()) != nullptr);
}

TEST_F(RegisteredClientStateTest, AnsweringMessageTransitionsToAnsweringState) {
    Message msg  = createMessage(MessageType::Answering);
    auto    next = state->transition(msg);
    ASSERT_NE(next, nullptr);
    EXPECT_TRUE(dynamic_cast<AnsweringState*>(next.get()) != nullptr);
}

TEST_F(RegisteredClientStateTest, UnknownMessageReturnsNullptr) {
    Message msg  = createMessage(MessageType::Text);
    auto    next = state->transition(msg);
    EXPECT_EQ(next, nullptr);
}

TEST_F(RegisteredClientStateTest, CallingWithEmptyFromUserStillTransitions) {
    Message msg  = createMessage(MessageType::Calling, "");
    auto    next = state->transition(msg);
    ASSERT_NE(next, nullptr);
    EXPECT_TRUE(dynamic_cast<CallingState*>(next.get()) != nullptr);
}

TEST_F(RegisteredClientStateTest, ManyUnknownMessagesReturnNullptr) {
    std::vector<MessageType> unknowns = {MessageType::Register,
                                         MessageType::Call,
                                         MessageType::Exit,
                                         MessageType::End,
                                         MessageType::Text};

    for (auto type : unknowns) {
        Message msg  = createMessage(type);
        auto    next = state->transition(msg);
        EXPECT_EQ(next, nullptr) << "Failed for type: " << type;
    }
}
