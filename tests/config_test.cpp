#include <sightline/config.h>

#include <gtest/gtest.h>

TEST(Config, DefaultTickRateIs128) {
    EXPECT_EQ(sightline::ServerConfig{}.tick_rate, 128);
}

TEST(Config, TickDurationMatchesTickRate) {
    EXPECT_DOUBLE_EQ(sightline::tick_duration_ms({.tick_rate = 128}), 7.8125);
    EXPECT_DOUBLE_EQ(sightline::tick_duration_ms({.tick_rate = 64}), 15.625);
    EXPECT_DOUBLE_EQ(sightline::tick_duration_ms({.tick_rate = 32}), 31.25);
}
