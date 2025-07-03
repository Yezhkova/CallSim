#include "turnstile.h"

#include <gtest/gtest.h>
// #include "turnstileSwitch.h"

TEST(TurnstileTest, CoinUnlocks) {
    EXPECT_EQ(next(State::Locked, Event::Coin), State::Unlocked);
}

TEST(TurnstileTest, PushLockedStaysLocked) {
    EXPECT_THROW(next(State::Locked, Event::Push), std::runtime_error);
}

TEST(TurnstileTest, PushFromUnlockedLocks) {
    EXPECT_EQ(next(State::Unlocked, Event::Push), State::Locked);
}

TEST(TurnstileTest, CoinFromUnlockedStaysUnlocked) {
    EXPECT_THROW(next(State::Unlocked, Event::Coin), std::runtime_error);
}

TEST(TurnstileTest, InvalidTransitionMessage) {
    try {
        next(static_cast<State>(-1), Event::Coin);
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error& e) {
        EXPECT_STREQ(e.what(), "Invalid transition");
    } catch (...) {
        FAIL() << "Expected std::runtime_error";
    }
}
