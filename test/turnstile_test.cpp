#include <gtest/gtest.h>
#include "turnstile.h"

TEST(TurnstileTest, CoinUnlocks) {
    EXPECT_EQ(next(Locked, Coin), Unlocked);
}

TEST(TurnstileTest, PushLockedStaysLocked) {
    EXPECT_THROW(next(Locked, Push), std::runtime_error);
}

TEST(TurnstileTest, PushFromUnlockedLocks) {
    EXPECT_EQ(next(Unlocked, Push), Locked);
}

TEST(TurnstileTest, CoinFromUnlockedStaysUnlocked) {
    EXPECT_THROW(next(Unlocked, Coin), std::runtime_error);
}

TEST(TurnstileTest, InvalidTransitionMessage) {
    try {
        next(static_cast<State>(-1), Coin);
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error& e) {
        EXPECT_STREQ(e.what(), "Invalid transition");
    } catch (...) {
        FAIL() << "Expected std::runtime_error";
    }
}
