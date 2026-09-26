#include <gtest/gtest.h>
#include <iostream>
#include <random>

#include "../src/engine/ApplyOutcome.h"
#include "../src/engine/GameState.h"
#include "../src/engine/OutcomeSampler.h"
#include "../src/engine/TeamProfile.h"

namespace {
    const char* toString(gridiron::PlayType type) {
        switch (type) {
            case gridiron::PlayType::Run: return "Run";
            case gridiron::PlayType::Pass: return "Pass";
            case gridiron::PlayType::FieldGoalAttempt: return "FieldGoalAttempt";
        }
        return "Unknown";
    }

    const char* toString(gridiron::OutcomeType type) {
        switch (type) {
            case gridiron::OutcomeType::Normal: return "Normal";
            case gridiron::OutcomeType::Turnover: return "Turnover";
            case gridiron::OutcomeType::FieldGoalMade: return "FieldGoalMade";
        }
        return "Unknown";
    }
}

// M1.1 shared checkpoint: sample one real event with samplePlayOutcome,
// apply it with applyOutcome, and inspect the resulting state.
TEST(IntegrationTest, SampleOneEventAndApplyIt) {
    gridiron::TeamProfile offense{"BAL", 0.8, 0.7, 0.8, 0.7, 0.05, 0.8};
    gridiron::TeamProfile defense{"SEA", 0.7, 0.9, 0.9, 0.5, 0.05, 0.8};

    gridiron::GameState state;
    state.home = offense;
    state.away = defense;

    std::mt19937 rng(2000);
    gridiron::PlayOutcome outcome = gridiron::samplePlayOutcome(state, rng);
    gridiron::GameState newState = gridiron::applyOutcome(state, outcome);

    std::cout << "Sampled play: play_type=" << toString(outcome.play_type)
               << " outcome_type=" << toString(outcome.outcome_type)
               << " yards_gained=" << outcome.yards_gained
               << " elapsed_seconds=" << outcome.elapsed_seconds << "\n";
    std::cout << "Resulting state: quarter=" << newState.quarter
               << " time_remaining=" << newState.time_remaining_seconds
               << " down=" << newState.down
               << " distance=" << newState.distance
               << " ball_position=" << newState.ball_position
               << " home_score=" << newState.home_score
               << " away_score=" << newState.away_score << "\n";

    // Legal-state bounds, matching the acceptance criteria in docs/DECISIONS.md.
    EXPECT_GE(newState.down, 1);
    EXPECT_LE(newState.down, 4);
    EXPECT_GE(newState.ball_position, 0);
    EXPECT_LE(newState.ball_position, 100);
    EXPECT_GE(newState.time_remaining_seconds, 0);
    EXPECT_LE(newState.time_remaining_seconds, 900);
}

// A 4th-down snap in range must travel the field goal path end to end: the
// sampler decides to kick, and applyOutcome resolves it as points or a turnover.
TEST(IntegrationTest, FieldGoalAttemptRunsEndToEnd) {
    gridiron::TeamProfile kicking{.name = "BAL", .field_goal_rating = 0.95};
    gridiron::TeamProfile defending{.name = "SEA"};

    // 20 yards to the goal is a 37 yard kick, so a make and a miss leave the
    // ball on different yard lines (25 versus 20) and are distinguishable.
    const auto state = gridiron::GameState{
        .home = kicking, .away = defending, .down = 4, .ball_position = 80};

    std::mt19937 rng(7);
    const gridiron::PlayOutcome outcome = gridiron::samplePlayOutcome(state, rng);
    ASSERT_EQ(outcome.play_type, gridiron::PlayType::FieldGoalAttempt);

    const gridiron::GameState newState = gridiron::applyOutcome(state, outcome);

    EXPECT_EQ(newState.possession, gridiron::Possession::Away);
    EXPECT_EQ(newState.down, 1);
    EXPECT_EQ(newState.distance, 10);
    EXPECT_EQ(newState.away_score, 0);

    if (outcome.outcome_type == gridiron::OutcomeType::FieldGoalMade) {
        EXPECT_EQ(newState.home_score, 3);
        EXPECT_EQ(newState.ball_position, 25);
    } else {
        ASSERT_EQ(outcome.outcome_type, gridiron::OutcomeType::Turnover);
        EXPECT_EQ(newState.home_score, 0);
        EXPECT_EQ(newState.ball_position, 20);
    }
}
