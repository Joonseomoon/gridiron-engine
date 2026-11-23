#include <gtest/gtest.h>

#include "../src/engine/LateGamePolicy.h"

TEST(LateGamePolicyTest, TrailingOffesnseTimeoutAtOneMinute) {
    const gridiron::LateGamePolicy policy;
    gridiron::GameState state{};
    state.quarter = 4;
    state.possession = gridiron::Possession::Home;
    state.home_score = 24;
    state.away_score = 28;

    gridiron::PlayOutcome play{};
    play.elapsed_seconds = 6;

    state.time_remaining_seconds = 66;
    EXPECT_TRUE(policy.callTimeout(state, play, gridiron::Possession::Home));

    state.time_remaining_seconds = 67;
    EXPECT_FALSE(policy.callTimeout(state, play, gridiron::Possession::Home));
}

TEST(LateGamePolicyTest, TrailingDefenseTimeoutAtTwoMinute) {
    const gridiron::LateGamePolicy policy;
    gridiron::GameState state{};
    state.quarter = 4;
    state.possession = gridiron::Possession::Away;
    state.home_score = 24;
    state.away_score = 28;

    gridiron::PlayOutcome play{};
    play.elapsed_seconds = 6;

    state.time_remaining_seconds = 126;
    EXPECT_TRUE(policy.callTimeout(state, play, gridiron::Possession::Home));

    state.time_remaining_seconds = 127;
    EXPECT_FALSE(policy.callTimeout(state, play, gridiron::Possession::Home));
}

TEST(LateGamePolicyTest, TrailingAwayOffenseFourthDownAtTwoMinutes) {
    const gridiron::LateGamePolicy policy;
    const gridiron::Matchup teams{};
    std::mt19937 rng(42);
    gridiron::RandomSource random{rng};

    gridiron::GameState state{};
    state.quarter = 4;
    state.possession = gridiron::Possession::Away;
    state.down = 4;
    state.distance = 4;
    state.ball_position = 80;
    state.home_score = 28;
    state.away_score = 24;

    state.time_remaining_seconds = 121;
    EXPECT_EQ(policy.choosePlay(state, teams, random), gridiron::PlayType::FieldGoalAttempt);

    state.time_remaining_seconds = 120;
    const auto play = policy.choosePlay(state, teams, random);
    EXPECT_TRUE(play == gridiron::PlayType::Run || play == gridiron::PlayType::Pass);

    state.quarter = 3;
    EXPECT_EQ(policy.choosePlay(state, teams, random), gridiron::PlayType::FieldGoalAttempt);
}

TEST(LateGamePolicyTest, TrailingAwayOffenseFourthDownFieldGoalTie) {
    const gridiron::LateGamePolicy policy;
    const gridiron::Matchup teams{};
    std::mt19937 rng(42);
    gridiron::RandomSource random{rng};

    gridiron::GameState state{};
    state.quarter = 4;
    state.possession = gridiron::Possession::Away;
    state.down = 4;
    state.distance = 4;
    state.ball_position = 80;
    state.home_score = 28;
    state.away_score = 25;

    state.time_remaining_seconds = 121;
    EXPECT_EQ(policy.choosePlay(state, teams, random), gridiron::PlayType::FieldGoalAttempt);

    state.time_remaining_seconds = 120;
    EXPECT_EQ(policy.choosePlay(state, teams, random), gridiron::PlayType::FieldGoalAttempt);
}

TEST(LateGamePolicyTest, FourthDownAtEdgeOfFieldGoalRange) {
    const gridiron::LateGamePolicy policy;
    const gridiron::Matchup teams{};
    std::mt19937 rng(42);
    gridiron::RandomSource random{rng};

    gridiron::GameState state{};
    state.quarter = 4;
    state.possession = gridiron::Possession::Away;
    state.down = 4;
    state.distance = 4;
    state.home_score = 28;
    state.away_score = 24;
    state.time_remaining_seconds = 120;

    state.ball_position = 60;
    const auto play = policy.choosePlay(state, teams, random);
    EXPECT_TRUE(play == gridiron::PlayType::Run || play == gridiron::PlayType::Pass);

    state.ball_position = 59;
    EXPECT_EQ(policy.choosePlay(state, teams, random), gridiron::PlayType::Punt);
}

TEST(LateGamePolicyTest, TrailingOffesnseTimeoutDependsOnQuarter) {
    const gridiron::LateGamePolicy policy;

    gridiron::GameState state{};
    state.quarter = 4;
    state.possession = gridiron::Possession::Away;
    state.down = 4;
    state.distance = 4;
    state.home_score = 28;
    state.away_score = 24;
    state.time_remaining_seconds = 96;
    state.ball_position = 60;

    gridiron::PlayOutcome play{};
    play.elapsed_seconds = 9;

    EXPECT_FALSE(policy.callTimeout(state, play, gridiron::Possession::Away));
    state.quarter = 2;
    EXPECT_TRUE(policy.callTimeout(state, play, gridiron::Possession::Away));
    state.quarter = 5;
    EXPECT_TRUE(policy.callTimeout(state, play, gridiron::Possession::Away));
}

TEST(LateGamePolicyTest, TimeoutRequiresTimeRemainingAndTrailingScore) {
    const gridiron::LateGamePolicy policy;

    gridiron::GameState state{};
    state.quarter = 4;
    state.possession = gridiron::Possession::Home;
    state.down = 4;
    state.distance = 4;
    state.home_score = 23;
    state.away_score = 24;
    state.time_remaining_seconds = 6;
    state.ball_position = 60;

    gridiron::PlayOutcome play{};
    play.elapsed_seconds = 6;

    EXPECT_FALSE(policy.callTimeout(state, play, gridiron::Possession::Home));
    state.time_remaining_seconds = 7;
    EXPECT_TRUE(policy.callTimeout(state, play, gridiron::Possession::Home));

    state.home_score = 24;
    EXPECT_FALSE(policy.callTimeout(state, play, gridiron::Possession::Home));
    state.home_score = 25;
    EXPECT_FALSE(policy.callTimeout(state, play, gridiron::Possession::Home));
}
