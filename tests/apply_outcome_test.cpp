#include <gtest/gtest.h>
#include "../src/engine/ApplyOutcome.h"
#include "../src/engine/GameState.h"
#include "../src/engine/PlayOutcome.h"

TEST(ApplyOutcome, Run) {
    auto curr_state = gridiron::GameState();
    auto playOutcome = gridiron::PlayOutcome(gridiron::PlayType::Run, 6, 38);
    gridiron::GameState newState = applyOutcome(curr_state, playOutcome);

    EXPECT_EQ(newState.quarter, 1);
    EXPECT_EQ(newState.time_remaining_seconds, 862);
    EXPECT_EQ(newState.down, 2);
    EXPECT_EQ(newState.distance, 4);
    EXPECT_EQ(newState.ball_position, 31);
    EXPECT_EQ(newState.home_score, 0);
    EXPECT_EQ(newState.away_score, 0);
    EXPECT_FALSE(newState.game_over);
}