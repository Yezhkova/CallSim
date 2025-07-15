#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "message.pb.h"
#include "mockSession.h"
#include "sessionFsm.h"

using ::testing::_;
using ::testing::Property;
using ::testing::Return;
using ::testing::StrictMock;

using namespace ses;

class ConnectedServerStateTest : public ::testing::Test {
   protected:
    std::shared_ptr<StrictMock<MockSession>> session;
    StateMachine                        fsm;
    std::unique_ptr<IState>             state;

    void SetUp() override {
        session = std::make_shared<StrictMock<MockSession>>();
        ON_CALL(*session, getData())
            .WillByDefault(Return("127.0.0.1:1111"));
        state = std::make_unique<ConnectedState>(session, fsm);
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
    EXPECT_CALL(*session, getData()).Times(2);

    Message msg  = makeMessage(MessageType::Register, "alice");
    auto    next = state->transition(msg);

    ASSERT_NE(next, nullptr);
    EXPECT_TRUE(dynamic_cast<RegisteredState*>(next.get()));
}

TEST_F(ConnectedServerStateTest, RegisterFailsStaysInConnected) {
    EXPECT_CALL(*session, registerClient("bob")).WillOnce(Return(false));
    EXPECT_CALL(*session, getData()).Times(2);

    Message msg  = makeMessage(MessageType::Register, "bob");
    auto    next = state->transition(msg);

    ASSERT_NE(next, nullptr);
    EXPECT_TRUE(dynamic_cast<ConnectedState*>(next.get()));
}

TEST_F(ConnectedServerStateTest, ExitClosesSessionAndReturnsNull) {
    EXPECT_CALL(*session, close()).Times(1);

    Message msg  = makeMessage(MessageType::Exit);
    auto    next = state->transition(msg);

    EXPECT_EQ(next, nullptr);
}

TEST_F(ConnectedServerStateTest, UnknownMessageReturnsNullptr) {
    Message msg  = makeMessage(MessageType::Call);
    auto    next = state->transition(msg);
    EXPECT_EQ(next, nullptr);
}

TEST_F(ConnectedServerStateTest, ThrowsIfSessionExpired) {
    std::shared_ptr<ISession> tmp_session =
        std::make_shared<StrictMock<MockSession>>();
    std::weak_ptr<ISession> expired = tmp_session;
    auto state = std::make_unique<RegisteredState>(tmp_session, fsm);
    tmp_session.reset();

    Message msg = makeMessage(MessageType::Register, "alice");

    EXPECT_THROW({ state->transition(msg); }, NullSessionException);
}

TEST_F(ConnectedServerStateTest, RegisterWithEmptyFromUserFails) {
    EXPECT_CALL(*session, registerClient("")).WillOnce(Return(false));
    EXPECT_CALL(*session, getData()).Times(2);

    Message msg  = makeMessage(MessageType::Register, "");
    auto    next = state->transition(msg);

    ASSERT_NE(next, nullptr);
    EXPECT_TRUE(dynamic_cast<ConnectedState*>(next.get()));
}

TEST_F(ConnectedServerStateTest, RepeatRegisterFromSameUserIsDenied) {
    Message msg = makeMessage(MessageType::Register, "alice");

    EXPECT_CALL(*session, getData()).Times(2);
    EXPECT_CALL(*session, registerClient("alice")).WillOnce(Return(true));

    auto first_transition = state->transition(msg);
    ASSERT_NE(first_transition, nullptr);
    EXPECT_TRUE(dynamic_cast<RegisteredState*>(first_transition.get()));

    auto second_state = std::make_unique<ConnectedState>(session, fsm);

    Message expected;
    expected.set_type(MessageType::Rejected);
    expected.set_to_user("alice");

    EXPECT_CALL(
        *session,
        sendMessageToClient(Property(&Message::type, MessageType::Rejected)))
        .Times(1);

    EXPECT_CALL(*session, getData()).Times(2);

    EXPECT_CALL(*session, registerClient("alice"))
        .WillOnce([&](const std::string& name) {
            Message msg;
            msg.set_type(MessageType::Rejected);
            msg.set_to_user(name);
            session->sendMessageToClient(msg);
            return false;
        });

    auto second_transition = second_state->transition(msg);
    ASSERT_NE(second_transition, nullptr);
    EXPECT_TRUE(dynamic_cast<ConnectedState*>(second_transition.get()));
}
