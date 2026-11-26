#include <gtest/gtest.h>

#include "../src/engine/LateGamePolicy.h"

TEST(LateGamePolicyTest, TrailingOffenseTimeoutAtOneMinute) {
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

    // Outside field-goal range a trailing team still goes for it instead of punting.
    state.ball_position = 59;
    const auto outside = policy.choosePlay(state, teams, random);
    EXPECT_TRUE(outside == gridiron::PlayType::Run || outside == gridiron::PlayType::Pass);
}

// Down 7, 0:40 left, 4th and 8 at its own 30: go for it, never punt.
TEST(LateGamePolicyTest, TrailingLateDeepInOwnTerritoryGoesForIt) {
    const gridiron::LateGamePolicy policy;
    const gridiron::Matchup teams{};
    std::mt19937 rng(42);
    gridiron::RandomSource random{rng};

    gridiron::GameState state{};
    state.quarter = 4;
    state.time_remaining_seconds = 40;
    state.possession = gridiron::Possession::Home;
    state.down = 4;
    state.distance = 8;
    state.ball_position = 30;
    state.home_score = 17;
    state.away_score = 24;

    const auto play = policy.choosePlay(state, teams, random);
    EXPECT_TRUE(play == gridiron::PlayType::Run || play == gridiron::PlayType::Pass);

    // Not trailing: back to the default punt.
    state.home_score = 24;
    EXPECT_EQ(policy.choosePlay(state, teams, random), gridiron::PlayType::Punt);
}

// The M4.md example (3:14 left) is outside the 2:00 window, so the default rule applies: 4th and 2 means go for it.
TEST(LateGamePolicyTest, ExampleScenarioUsesTheDefaultRule) {
    const gridiron::LateGamePolicy policy;
    const gridiron::Matchup teams{};
    std::mt19937 rng(42);
    gridiron::RandomSource random{rng};

    const gridiron::GameState state{.quarter = 4,
                                    .time_remaining_seconds = 194,
                                    .possession = gridiron::Possession::Home,
                                    .down = 4,
                                    .distance = 2,
                                    .ball_position = 57,
                                    .home_score = 24,
                                    .away_score = 27};
    const auto play = policy.choosePlay(state, teams, random);
    EXPECT_TRUE(play == gridiron::PlayType::Run || play == gridiron::PlayType::Pass);
}

// The defense only calls a timeout when trailing and with time still left after the play.
TEST(LateGamePolicyTest, DefenseTimeoutNeedsDeficitAndTimeLeft) {
    const gridiron::LateGamePolicy policy;
    gridiron::GameState state{};
    state.quarter = 4;
    state.possession = gridiron::Possession::Away;
    state.home_score = 24;
    state.away_score = 28;

    gridiron::PlayOutcome play{};
    play.elapsed_seconds = 6;

    state.time_remaining_seconds = 6;
    EXPECT_FALSE(policy.callTimeout(state, play, gridiron::Possession::Home));
    state.time_remaining_seconds = 7;
    EXPECT_TRUE(policy.callTimeout(state, play, gridiron::Possession::Home));

    state.home_score = 28;
    EXPECT_FALSE(policy.callTimeout(state, play, gridiron::Possession::Home));
}

TEST(LateGamePolicyTest, TrailingOffenseTimeoutDependsOnQuarter) {
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
