#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "message.pb.h"
#include "mockSession.h"
#include "sessionFsm.h"

using ::testing::_;
using ::testing::Return;
using ::testing::StrictMock;
using namespace ses;

class RegisteredServerStateTest : public ::testing::Test {
   protected:
    std::shared_ptr<StrictMock<MockSession>> mockSession;
    StateMachine                             fsm;

    void SetUp() override {
        mockSession = std::make_shared<StrictMock<MockSession>>();
    }

    Message createMessage(MessageType        type,
                          const std::string& from = "alice",
                          const std::string& to   = "bob") {
        Message msg;
        msg.set_type(type);
        msg.set_from_user(from);
        msg.set_to_user(to);
        return msg;
    }
};

TEST_F(RegisteredServerStateTest, CallAcceptedTransitionsToCallingState) {
    auto msg = createMessage(MessageType::Call);
    EXPECT_CALL(*mockSession, getEndpoint())
        .WillRepeatedly(Return("127.0.0.1:12345"));
    EXPECT_CALL(*mockSession, callClient("alice", "bob"))
        .WillOnce(Return(true));
    EXPECT_CALL(*mockSession, sendMessageToClient(_));  // callConfirmed

    auto state = RegisteredState::create(mockSession, fsm);
    auto next  = state->transition(msg);

    ASSERT_NE(next, nullptr);
    EXPECT_TRUE(dynamic_cast<CallingState*>(next.get()));
}

TEST_F(RegisteredServerStateTest, CallRejectedStaysInRegisteredState) {
    auto msg = createMessage(MessageType::Call);
    EXPECT_CALL(*mockSession, getEndpoint())
        .WillRepeatedly(Return("127.0.0.1:12345"));
    EXPECT_CALL(*mockSession, callClient(_, _)).WillOnce(Return(false));
    EXPECT_CALL(*mockSession, sendMessageToClient(_));  // callDenied

    auto state = RegisteredState::create(mockSession, fsm);
    auto next  = state->transition(msg);

    ASSERT_NE(next, nullptr);
    EXPECT_TRUE(dynamic_cast<RegisteredState*>(next.get()));
}

TEST_F(RegisteredServerStateTest, AnswerMessageTransitionsToAnsweringState) {
    auto msg = createMessage(MessageType::Answer);
    EXPECT_CALL(*mockSession, getEndpoint())
        .WillRepeatedly(Return("127.0.0.1:12345"));
    EXPECT_CALL(*mockSession, sendMessageToClient(_));  // answerConfirmed

    auto state = RegisteredState::create(mockSession, fsm);
    auto next  = state->transition(msg);

    ASSERT_NE(next, nullptr);
    EXPECT_TRUE(dynamic_cast<AnsweringState*>(next.get()));
}

TEST_F(RegisteredServerStateTest, ExitDeletesClientAndReturnsNullptr) {
    auto msg = createMessage(MessageType::Exit);
    EXPECT_CALL(*mockSession, getEndpoint())
        .WillOnce(::testing::Return("127.0.0.1:12345"));
    EXPECT_CALL(*mockSession, deleteClient("alice")).Times(1);

    auto state = RegisteredState::create(mockSession, fsm);
    auto next  = state->transition(msg);

    EXPECT_EQ(next, nullptr);
}

TEST_F(RegisteredServerStateTest, UnknownMessageReturnsNullptr) {
    auto msg = createMessage(MessageType::Text);
    EXPECT_CALL(*mockSession, getEndpoint())
        .WillOnce(::testing::Return("127.0.0.1:12345"));

    auto state = RegisteredState::create(mockSession, fsm);
    auto next  = state->transition(msg);

    EXPECT_EQ(next, nullptr);
}
