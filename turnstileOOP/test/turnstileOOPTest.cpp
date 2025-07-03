#include <gtest/gtest.h>

#include "turnstileOOP.h"

TEST(TurnstileOOPTest, LockedToUnlocked) {
    StateMachine fsm;
    EXPECT_NO_THROW(fsm.next(Event::Coin));
}

TEST(TurnstileOOPTest, LockedPushThrows) {
    StateMachine fsm;
    EXPECT_THROW(fsm.next(Event::Push), std::runtime_error);
}

TEST(TurnstileOOPTest, UnlockedToLocked) {
    StateMachine fsm;
    fsm.next(Event::Coin);
    EXPECT_NO_THROW(fsm.next(Event::Push));
}

TEST(TurnstileOOPTest, UnlockedCoinThrows) {
    StateMachine fsm;
    fsm.next(Event::Coin);
    EXPECT_THROW(fsm.next(Event::Coin), std::runtime_error);
}
