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
            case gridiron::OutcomeType::Touchdown: return "Touchdown";
            case gridiron::OutcomeType::Safety: return "Safety";
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
    gridiron::PlayOutcome outcome = gridiron::samplePlayOutcome(offense, defense, rng);
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

// Same seed, same teams: the sampled play and resulting state must be identical.
TEST(IntegrationTest, SameSeedProducesIdenticalResult) {
    gridiron::TeamProfile offense{"BAL", 0.8, 0.7, 0.8, 0.7, 0.05, 0.8};
    gridiron::TeamProfile defense{"SEA", 0.7, 0.9, 0.9, 0.5, 0.05, 0.8};

    gridiron::GameState state;
    state.home = offense;
    state.away = defense;

    std::mt19937 rngOne(1000);
    std::mt19937 rngTwo(1000);

    gridiron::PlayOutcome outcomeOne = gridiron::samplePlayOutcome(offense, defense, rngOne);
    gridiron::PlayOutcome outcomeTwo = gridiron::samplePlayOutcome(offense, defense, rngTwo);

    gridiron::GameState stateOne = gridiron::applyOutcome(state, outcomeOne);
    gridiron::GameState stateTwo = gridiron::applyOutcome(state, outcomeTwo);

    EXPECT_EQ(stateOne.down, stateTwo.down);
    EXPECT_EQ(stateOne.distance, stateTwo.distance);
    EXPECT_EQ(stateOne.ball_position, stateTwo.ball_position);
    EXPECT_EQ(stateOne.time_remaining_seconds, stateTwo.time_remaining_seconds);
    EXPECT_EQ(stateOne.home_score, stateTwo.home_score);
    EXPECT_EQ(stateOne.away_score, stateTwo.away_score);
}
