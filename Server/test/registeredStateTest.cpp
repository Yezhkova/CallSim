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
    EXPECT_CALL(*mockSession, getData())
        .WillRepeatedly(Return("127.0.0.1:12345"));
    EXPECT_CALL(*mockSession, callClient("alice", "bob"))
        .WillOnce(Return(true));
    EXPECT_CALL(*mockSession, sendMessageToClient(_));

    auto state = RegisteredState::create(mockSession, fsm);
    auto next  = state->transition(msg);

    ASSERT_NE(next, nullptr);
    EXPECT_TRUE(dynamic_cast<CallingState*>(next.get()));
}

TEST_F(RegisteredServerStateTest, CallRejectedStaysInRegisteredState) {
    auto msg = createMessage(MessageType::Call);
    EXPECT_CALL(*mockSession, getData())
        .WillRepeatedly(Return("127.0.0.1:12345"));
    EXPECT_CALL(*mockSession, callClient(_, _)).WillOnce(Return(false));
    EXPECT_CALL(*mockSession, sendMessageToClient(_));

    auto state = RegisteredState::create(mockSession, fsm);
    auto next  = state->transition(msg);

    EXPECT_TRUE(next == nullptr);
}

TEST_F(RegisteredServerStateTest, AnswerMessageTransitionsToAnsweringState) {
    auto msg = createMessage(MessageType::Answer);
    EXPECT_CALL(*mockSession, getData())
        .WillRepeatedly(Return("127.0.0.1:12345"));
    EXPECT_CALL(*mockSession, sendMessageToClient(_));

    auto state = RegisteredState::create(mockSession, fsm);
    auto next  = state->transition(msg);

    ASSERT_NE(next, nullptr);
    EXPECT_TRUE(dynamic_cast<AnsweringState*>(next.get()));
}

TEST_F(RegisteredServerStateTest, ExitDeletesClientAndReturnsNullptr) {
    auto msg = createMessage(MessageType::Exit);
    EXPECT_CALL(*mockSession, getData())
        .Times(2)
        .WillRepeatedly(::testing::Return("127.0.0.1:12345"));
    EXPECT_CALL(*mockSession, deleteClient("alice")).Times(1);
    EXPECT_CALL(*mockSession, close()).Times(1);

    auto state = RegisteredState::create(mockSession, fsm);
    auto next  = state->transition(msg);

    EXPECT_EQ(next, nullptr);
}

TEST_F(RegisteredServerStateTest, UnknownMessageReturnsNullptr) {
    auto msg = createMessage(MessageType::Text);

    EXPECT_CALL(*mockSession, getData()).WillOnce(Return("127.0.0.1:12345"));

    EXPECT_CALL(*mockSession,
                sendMessageToClient(::testing::Truly([](const Message& msg) {
                    return msg.payload().find("Invalid transition") !=
                           std::string::npos;
                })));

    auto state = RegisteredState::create(mockSession, fsm);
    auto next  = state->transition(msg);

    EXPECT_EQ(next, nullptr);
}

TEST_F(RegisteredServerStateTest, ThrowsIfSessionExpired) {
    std::shared_ptr<ISession> tmp_session =
        std::make_shared<StrictMock<MockSession>>();
    std::weak_ptr<ISession> expired = tmp_session;
    auto state = std::make_unique<RegisteredState>(tmp_session, fsm);
    tmp_session.reset();

    Message msg = createMessage(MessageType::Call);

    EXPECT_THROW({ state->transition(msg); }, NullSessionException);
}

TEST_F(RegisteredServerStateTest, CallWithEmptyToUserIsRejected) {
    Message msg = createMessage(MessageType::Call);
    msg.set_to_user("");

    EXPECT_CALL(*mockSession, getData())
        .WillRepeatedly(Return("127.0.0.1:12345"));
    EXPECT_CALL(*mockSession, callClient(_, _)).WillOnce(Return(false));
    EXPECT_CALL(*mockSession, sendMessageToClient(_));

    auto state = RegisteredState::create(mockSession, fsm);
    auto next  = state->transition(msg);

    ASSERT_NE(next, nullptr);
    EXPECT_TRUE(dynamic_cast<RegisteredState*>(next.get()));
}
using ::testing::Field;
using ::testing::Property;
using ::testing::Truly;

TEST_F(RegisteredServerStateTest, SendsCallConfirmedWithCorrectToUser) {
    auto msg = createMessage(MessageType::Call, "alice", "bob");

    EXPECT_CALL(*mockSession, getData())
        .WillRepeatedly(Return("127.0.0.1:12345"));
    EXPECT_CALL(*mockSession, callClient("alice", "bob"))
        .WillOnce(Return(true));
    EXPECT_CALL(
        *mockSession,
        sendMessageToClient(Property(&Message::type, MessageType::Calling)));

    auto state = RegisteredState::create(mockSession, fsm);
    auto next  = state->transition(msg);

    ASSERT_NE(next, nullptr);
    EXPECT_TRUE(dynamic_cast<CallingState*>(next.get()));
}
TEST_F(RegisteredServerStateTest, MultipleTransitionsHandleCorrectly) {
    auto msg1 = createMessage(MessageType::Call, "alice", "bob");
    auto msg2 = createMessage(MessageType::Answer, "bob", "alice");

    EXPECT_CALL(*mockSession, getData())
        .WillRepeatedly(Return("127.0.0.1:12345"));
    EXPECT_CALL(*mockSession, callClient("alice", "bob"))
        .WillOnce(Return(true));
    EXPECT_CALL(*mockSession, sendMessageToClient(_)).Times(1);

    auto state = RegisteredState::create(mockSession, fsm);
    auto next1 = state->transition(msg1);

    ASSERT_NE(next1, nullptr);
    auto next2 = next1->transition(msg2);
}
