#include <gtest/gtest.h>
#include <iostream>
#include <random>

#include "../src/engine/ApplyOutcome.h"
#include "../src/engine/CandidateAction.h"
#include "../src/engine/DefaultPolicy.h"
#include "../src/engine/GameRunner.h"
#include "../src/engine/GameState.h"
#include "../src/engine/LateGamePolicy.h"
#include "../src/engine/OutcomeSampler.h"
#include "../src/engine/PlayResult.h"
#include "../src/engine/RandomSource.h"
#include "../src/engine/ScenarioValidation.h"
#include "../src/engine/TeamProfile.h"

namespace {
const char* toString(gridiron::PlayType type) {
    switch (type) {
    case gridiron::PlayType::Run: return "Run";
    case gridiron::PlayType::Pass: return "Pass";
    case gridiron::PlayType::FieldGoalAttempt: return "FieldGoalAttempt";
    case gridiron::PlayType::Punt: return "Punt";
    }
    return "Unknown";
}

const char* toString(gridiron::OutcomeType type) {
    switch (type) {
    case gridiron::OutcomeType::Normal: return "Normal";
    case gridiron::OutcomeType::Turnover: return "Turnover";
    case gridiron::OutcomeType::FieldGoalMade: return "FieldGoalMade";
    case gridiron::OutcomeType::Incomplete: return "Incomplete";
    case gridiron::OutcomeType::Punted: return "Punted";
    }
    return "Unknown";
}
} // namespace

// M1.1 shared checkpoint: sample one real event with samplePlayOutcome,
// apply it with applyOutcome, and inspect the resulting state.
TEST(IntegrationTest, SampleOneEventAndApplyIt) {
    gridiron::TeamProfile offense{"BAL", 0.8, 0.7, 0.8, 0.7, 0.05, 0.8};
    gridiron::TeamProfile defense{"SEA", 0.7, 0.9, 0.9, 0.5, 0.05, 0.8};
    gridiron::Matchup matchup{.home = offense, .away = defense};

    gridiron::GameState state;

    std::mt19937 rng(2000);

    gridiron::RandomSource random{rng};
    const gridiron::DefaultPolicy policy;
    const gridiron::PlayType playType = policy.choosePlay(state, matchup, random);
    gridiron::PlayOutcome outcome = gridiron::samplePlayOutcome(state, matchup, playType, random);
    auto result = gridiron::applyOutcome(state, outcome);
    auto& newState = result.state;

    std::cout << "Sampled play: play_type=" << toString(outcome.play_type)
              << " outcome_type=" << toString(outcome.outcome_type) << " yards_gained=" << outcome.yards_gained
              << " elapsed_seconds=" << outcome.elapsed_seconds << "\n";
    std::cout << "Resulting state: quarter=" << newState.quarter
              << " time_remaining=" << newState.time_remaining_seconds << " down=" << newState.down
              << " distance=" << newState.distance << " ball_position=" << newState.ball_position
              << " home_score=" << newState.home_score << " away_score=" << newState.away_score << "\n";

    // Legal-state bounds, matching the acceptance criteria in docs/decisions/M0.md.
    EXPECT_GE(newState.down, 1);
    EXPECT_LE(newState.down, 4);
    // At or past either goal line always resolves as a score or a touchback,
    // so a live ball never rests on 0 or 100.
    EXPECT_GE(newState.ball_position, 1);
    EXPECT_LE(newState.ball_position, 99);
    EXPECT_GE(newState.time_remaining_seconds, 0);
    EXPECT_LE(newState.time_remaining_seconds, 900);
}

// A 4th-down snap in range must travel the field goal path end to end: the
// sampler decides to kick, and applyOutcome resolves it as points or a turnover.
TEST(IntegrationTest, FieldGoalAttemptRunsEndToEnd) {
    gridiron::TeamProfile kicking{.name = "BAL", .field_goal_rating = 0.95};
    gridiron::TeamProfile defending{.name = "SEA"};
    gridiron::Matchup matchup{.home = kicking, .away = defending};

    // 20 yards to the goal is a 37 yard kick, so a make and a miss leave the
    // ball on different yard lines (25 versus 20) and are distinguishable.
    const auto state = gridiron::GameState{.down = 4, .ball_position = 80};

    std::mt19937 rng(7);

    gridiron::RandomSource random{rng};
    const gridiron::DefaultPolicy policy;
    const gridiron::PlayType playType = policy.choosePlay(state, matchup, random);
    gridiron::PlayOutcome outcome = gridiron::samplePlayOutcome(state, matchup, playType, random);
    ASSERT_EQ(outcome.play_type, gridiron::PlayType::FieldGoalAttempt);

    auto result = gridiron::applyOutcome(state, outcome);
    auto& newState = result.state;

    EXPECT_EQ(newState.possession, gridiron::Possession::Away);
    EXPECT_EQ(newState.down, 1);
    EXPECT_EQ(newState.distance, 10);
    EXPECT_EQ(newState.away_score, 0);

    if (outcome.outcome_type == gridiron::OutcomeType::FieldGoalMade) {
        EXPECT_EQ(newState.home_score, 3);
        EXPECT_EQ(newState.ball_position, 35);
    } else {
        ASSERT_EQ(outcome.outcome_type, gridiron::OutcomeType::Turnover);
        EXPECT_EQ(newState.home_score, 0);
        EXPECT_EQ(newState.ball_position, 20);
    }
}

// M4.2 shared checkpoint: resume the M4.md example (BAL 24-27, 3:14 left, 4th and 2 at SEA's 43)
// under two forced actions and finish every game. Policies decide after the first play.
TEST(IntegrationTest, ResumeExampleScenarioUnderTwoActions) {
    gridiron::TeamProfile bal{"BAL", 0.8, 0.7, 0.8, 0.7, 0.05, 0.8};
    gridiron::TeamProfile sea{"SEA", 0.7, 0.9, 0.9, 0.5, 0.05, 0.8};
    const gridiron::Matchup matchup{.home = bal, .away = sea};
    const gridiron::GameState start{.quarter = 4,
                                    .time_remaining_seconds = 194,
                                    .possession = gridiron::Possession::Home,
                                    .down = 4,
                                    .distance = 2,
                                    .ball_position = 57,
                                    .home_score = 24,
                                    .away_score = 27};
    ASSERT_TRUE(gridiron::validateScenario(start).empty());

    const gridiron::LateGamePolicy policy;
    const gridiron::Policies policies{policy, policy};

    constexpr unsigned kBatchSeed = 1000;
    constexpr int kGames = 1000;
    for (const auto action : {gridiron::CandidateAction::GoForIt, gridiron::CandidateAction::FieldGoal}) {
        int finished = 0;
        double balWinValue = 0.0;
        for (int i = 0; i < kGames; ++i) {
            std::mt19937 rng(kBatchSeed + static_cast<unsigned>(i));
            const gridiron::GameState final = gridiron::resumeGame(start, matchup, policies, action, rng);
            if (final.game_over) {
                ++finished;
            }
            if (final.home_score > final.away_score) {
                balWinValue += 1.0;
            } else if (final.home_score == final.away_score) {
                balWinValue += 0.5;
            }
        }
        EXPECT_EQ(finished, kGames);
        std::cout << (action == gridiron::CandidateAction::GoForIt ? "GoForIt" : "FieldGoal") << ": BAL win value "
                  << balWinValue / kGames << " over " << kGames << " games\n";
    }
}
