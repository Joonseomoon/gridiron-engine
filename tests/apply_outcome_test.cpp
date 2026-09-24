#include <gtest/gtest.h>
#include "../src/engine/ApplyOutcome.h"
#include "../src/engine/GameState.h"
#include "../src/engine/PlayOutcome.h"

TEST(ApplyOutcomeTest, RunWithoutFirstDownAdvancesDownAndClock) {
    auto curr_state = gridiron::GameState();
    auto playOutcome = gridiron::PlayOutcome(gridiron::PlayType::Run, gridiron::OutcomeType::Normal, 6, 38);
    gridiron::GameState newState = applyOutcome(curr_state, playOutcome);

    EXPECT_EQ(newState.quarter, 1);
    EXPECT_EQ(newState.time_remaining_seconds, 862);
    EXPECT_EQ(newState.possession, gridiron::Possession::Home);
    EXPECT_EQ(newState.down, 2);
    EXPECT_EQ(newState.distance, 4);
    EXPECT_EQ(newState.ball_position, 31);
    EXPECT_EQ(newState.home_score, 0);
    EXPECT_EQ(newState.away_score, 0);
    EXPECT_FALSE(newState.game_over);
}

TEST(ApplyOutcomeTest, RunWithFirstDownConversionAdvancesClock) {
    auto curr_state = gridiron::GameState();
    auto playOutcome = gridiron::PlayOutcome(gridiron::PlayType::Run, gridiron::OutcomeType::Normal, 12, 38);
    gridiron::GameState newState = applyOutcome(curr_state, playOutcome);

    EXPECT_EQ(newState.quarter, 1);
    EXPECT_EQ(newState.time_remaining_seconds, 862);
    EXPECT_EQ(newState.possession, gridiron::Possession::Home);
    EXPECT_EQ(newState.down, 1);
    EXPECT_EQ(newState.distance, 10);
    EXPECT_EQ(newState.ball_position, 37);
    EXPECT_EQ(newState.home_score, 0);
    EXPECT_EQ(newState.away_score, 0);
    EXPECT_FALSE(newState.game_over);
}

TEST(ApplyOutcomeTest, TurnoverByHomeTeamWithPositiveYardsGained) {
    auto curr_state = gridiron::GameState();
    auto playOutcome = gridiron::PlayOutcome(gridiron::PlayType::Run, gridiron::OutcomeType::Turnover, 15, 38);
    gridiron::GameState newState = applyOutcome(curr_state, playOutcome);

    EXPECT_EQ(newState.quarter, 1);
    EXPECT_EQ(newState.time_remaining_seconds, 862);
    EXPECT_EQ(newState.possession, gridiron::Possession::Away);
    EXPECT_EQ(newState.down, 1);
    EXPECT_EQ(newState.distance, 10);
    EXPECT_EQ(newState.ball_position, 60);
    EXPECT_EQ(newState.home_score, 0);
    EXPECT_EQ(newState.away_score, 0);
    EXPECT_FALSE(newState.game_over);
}

TEST(ApplyOutcomeTest, TurnoverByHomeTeamWithNegativeYardsGained) {
    auto curr_state = gridiron::GameState();
    auto playOutcome = gridiron::PlayOutcome(gridiron::PlayType::Run, gridiron::OutcomeType::Turnover, -8, 38);
    gridiron::GameState newState = applyOutcome(curr_state, playOutcome);

    EXPECT_EQ(newState.quarter, 1);
    EXPECT_EQ(newState.time_remaining_seconds, 862);
    EXPECT_EQ(newState.possession, gridiron::Possession::Away);
    EXPECT_EQ(newState.down, 1);
    EXPECT_EQ(newState.distance, 10);
    EXPECT_EQ(newState.ball_position, 83);
    EXPECT_EQ(newState.home_score, 0);
    EXPECT_EQ(newState.away_score, 0);
    EXPECT_FALSE(newState.game_over);
}
TEST(ApplyOutcomeTest, TurnoverByAwayTeamWithPositiveYardsGained) {
    auto curr_state = gridiron::GameState{.possession = gridiron::Possession::Away};
    auto playOutcome = gridiron::PlayOutcome(gridiron::PlayType::Run, gridiron::OutcomeType::Turnover, 15, 38);
    gridiron::GameState newState = applyOutcome(curr_state, playOutcome);

    EXPECT_EQ(newState.quarter, 1);
    EXPECT_EQ(newState.time_remaining_seconds, 862);
    EXPECT_EQ(newState.possession, gridiron::Possession::Home);
    EXPECT_EQ(newState.down, 1);
    EXPECT_EQ(newState.distance, 10);
    EXPECT_EQ(newState.ball_position, 60);
    EXPECT_EQ(newState.home_score, 0);
    EXPECT_EQ(newState.away_score, 0);
    EXPECT_FALSE(newState.game_over);
}

TEST(ApplyOutcomeTest, TurnoverByAwayTeamWithNegativeYardsGained) {
    auto curr_state = gridiron::GameState{.possession = gridiron::Possession::Away};
    auto playOutcome = gridiron::PlayOutcome(gridiron::PlayType::Run, gridiron::OutcomeType::Turnover, -8, 38);
    gridiron::GameState newState = applyOutcome(curr_state, playOutcome);

    EXPECT_EQ(newState.quarter, 1);
    EXPECT_EQ(newState.time_remaining_seconds, 862);
    EXPECT_EQ(newState.possession, gridiron::Possession::Home);
    EXPECT_EQ(newState.down, 1);
    EXPECT_EQ(newState.distance, 10);
    EXPECT_EQ(newState.ball_position, 83);
    EXPECT_EQ(newState.home_score, 0);
    EXPECT_EQ(newState.away_score, 0);
    EXPECT_FALSE(newState.game_over);
}

TEST(ApplyOutcomeTest, AwayRunCrossesGoalLineScoresTouchdown) {
    auto curr_state = gridiron::GameState{.possession = gridiron::Possession::Away, .ball_position = 90};
    auto playOutcome = gridiron::PlayOutcome(gridiron::PlayType::Run, gridiron::OutcomeType::Normal, 15, 38);
    gridiron::GameState newState = applyOutcome(curr_state, playOutcome);

    EXPECT_EQ(newState.quarter, 1);
    EXPECT_EQ(newState.time_remaining_seconds, 862);
    EXPECT_EQ(newState.possession, gridiron::Possession::Home);
    EXPECT_EQ(newState.down, 1);
    EXPECT_EQ(newState.distance, 10);
    EXPECT_EQ(newState.ball_position, 25);
    EXPECT_EQ(newState.home_score, 0);
    EXPECT_EQ(newState.away_score, 7);
    EXPECT_FALSE(newState.game_over);
}

TEST(ApplyOutcomeTest, FieldGoalMissedByHomeTeam) {
    auto curr_state = gridiron::GameState();
    auto playOutcome = gridiron::PlayOutcome(gridiron::PlayType::FieldGoalAttempt, gridiron::OutcomeType::Turnover, 1000, 38);
    gridiron::GameState newState = applyOutcome(curr_state, playOutcome);

    EXPECT_EQ(newState.quarter, 1);
    EXPECT_EQ(newState.time_remaining_seconds, 862);
    EXPECT_EQ(newState.possession, gridiron::Possession::Away);
    EXPECT_EQ(newState.down, 1);
    EXPECT_EQ(newState.distance, 10);
    EXPECT_EQ(newState.ball_position, 75);
    EXPECT_EQ(newState.home_score, 0);
    EXPECT_EQ(newState.away_score, 0);
    EXPECT_FALSE(newState.game_over);
}

TEST(ApplyOutcomeTest, FieldGoalMadeByHomeTeam) {
    auto curr_state = gridiron::GameState();
    auto playOutcome = gridiron::PlayOutcome(gridiron::PlayType::FieldGoalAttempt, gridiron::OutcomeType::FieldGoalMade, 75, 38);
    gridiron::GameState newState = applyOutcome(curr_state, playOutcome);

    EXPECT_EQ(newState.quarter, 1);
    EXPECT_EQ(newState.time_remaining_seconds, 862);
    EXPECT_EQ(newState.possession, gridiron::Possession::Away);
    EXPECT_EQ(newState.down, 1);
    EXPECT_EQ(newState.distance, 10);
    EXPECT_EQ(newState.ball_position, 25);
    EXPECT_EQ(newState.home_score, 3);
    EXPECT_EQ(newState.away_score, 0);
    EXPECT_FALSE(newState.game_over);
}

TEST(ApplyOutcomeTest, FieldGoalMissedByAwayTeam) {
    auto curr_state = gridiron::GameState{.possession = gridiron::Possession::Away};
    auto playOutcome = gridiron::PlayOutcome(gridiron::PlayType::FieldGoalAttempt, gridiron::OutcomeType::Turnover, 75, 38);
    gridiron::GameState newState = applyOutcome(curr_state, playOutcome);

    EXPECT_EQ(newState.quarter, 1);
    EXPECT_EQ(newState.time_remaining_seconds, 862);
    EXPECT_EQ(newState.possession, gridiron::Possession::Home);
    EXPECT_EQ(newState.down, 1);
    EXPECT_EQ(newState.distance, 10);
    EXPECT_EQ(newState.ball_position, 75);
    EXPECT_EQ(newState.home_score, 0);
    EXPECT_EQ(newState.away_score, 0);
    EXPECT_FALSE(newState.game_over);
}

TEST(ApplyOutcomeTest, FieldGoalMadeByAwayTeam) {
    auto curr_state = gridiron::GameState{.possession = gridiron::Possession::Away};
    auto playOutcome = gridiron::PlayOutcome(gridiron::PlayType::FieldGoalAttempt, gridiron::OutcomeType::FieldGoalMade, 0, 38);
    gridiron::GameState newState = applyOutcome(curr_state, playOutcome);

    EXPECT_EQ(newState.quarter, 1);
    EXPECT_EQ(newState.time_remaining_seconds, 862);
    EXPECT_EQ(newState.possession, gridiron::Possession::Home);
    EXPECT_EQ(newState.down, 1);
    EXPECT_EQ(newState.distance, 10);
    EXPECT_EQ(newState.ball_position, 25);
    EXPECT_EQ(newState.home_score, 0);
    EXPECT_EQ(newState.away_score, 3);
    EXPECT_FALSE(newState.game_over);
}

TEST(ApplyOutcomeTest, AwayTackledBehindOwnGoalLineGivesHomeSafety) {
    auto curr_state = gridiron::GameState{.possession = gridiron::Possession::Away, .ball_position = 4};
    auto playOutcome = gridiron::PlayOutcome(gridiron::PlayType::Run, gridiron::OutcomeType::Normal, -7, 38);
    gridiron::GameState newState = applyOutcome(curr_state, playOutcome);

    EXPECT_EQ(newState.quarter, 1);
    EXPECT_EQ(newState.time_remaining_seconds, 862);
    EXPECT_EQ(newState.possession, gridiron::Possession::Home);
    EXPECT_EQ(newState.down, 1);
    EXPECT_EQ(newState.distance, 10);
    EXPECT_EQ(newState.ball_position, 25);
    EXPECT_EQ(newState.home_score, 2);
    EXPECT_EQ(newState.away_score, 0);
    EXPECT_FALSE(newState.game_over);
}

TEST(ApplyOutcomeTest, 4thDownFailConversionWithPass) {
    auto curr_state = gridiron::GameState{.down = 4};
    auto playOutcome = gridiron::PlayOutcome(gridiron::PlayType::Pass, gridiron::OutcomeType::Normal, 3, 38);
    gridiron::GameState newState = applyOutcome(curr_state, playOutcome);

    EXPECT_EQ(newState.quarter, 1);
    EXPECT_EQ(newState.time_remaining_seconds, 862);
    EXPECT_EQ(newState.possession, gridiron::Possession::Away);
    EXPECT_EQ(newState.down, 1);
    EXPECT_EQ(newState.distance, 10);
    EXPECT_EQ(newState.ball_position, 72);
    EXPECT_EQ(newState.home_score, 0);
    EXPECT_EQ(newState.away_score, 0);
    EXPECT_FALSE(newState.game_over);
}

TEST(ApplyOutcomeTest, RunWithNegativeYardsGainedBelowZero) {
    auto curr_state = gridiron::GameState{.ball_position = 4};
    auto playOutcome = gridiron::PlayOutcome(gridiron::PlayType::Run, gridiron::OutcomeType::Normal, -7, 38);
    gridiron::GameState newState = applyOutcome(curr_state, playOutcome);

    EXPECT_EQ(newState.quarter, 1);
    EXPECT_EQ(newState.time_remaining_seconds, 862);
    EXPECT_EQ(newState.possession, gridiron::Possession::Away);
    EXPECT_EQ(newState.down, 1);
    EXPECT_EQ(newState.distance, 10);
    EXPECT_EQ(newState.ball_position, 25);
    EXPECT_EQ(newState.home_score, 0);
    EXPECT_EQ(newState.away_score, 2);
    EXPECT_FALSE(newState.game_over);
}

TEST(ApplyOutcomeTest, 4thDownAndPassWithNegativeYardsGainedBelowZero) {
    auto curr_state = gridiron::GameState{.down = 4, .ball_position = 4};
    auto playOutcome = gridiron::PlayOutcome(gridiron::PlayType::Pass, gridiron::OutcomeType::Normal, -7, 38);
    gridiron::GameState newState = applyOutcome(curr_state, playOutcome);

    EXPECT_EQ(newState.quarter, 1);
    EXPECT_EQ(newState.time_remaining_seconds, 862);
    EXPECT_EQ(newState.possession, gridiron::Possession::Away);
    EXPECT_EQ(newState.down, 1);
    EXPECT_EQ(newState.distance, 10);
    EXPECT_EQ(newState.ball_position, 25);
    EXPECT_EQ(newState.home_score, 0);
    EXPECT_EQ(newState.away_score, 2);
    EXPECT_FALSE(newState.game_over);
}

TEST(ApplyOutcomeTest, RunWith1stQuarterEnd) {
    auto curr_state = gridiron::GameState{.time_remaining_seconds = 10};
    auto playOutcome = gridiron::PlayOutcome(gridiron::PlayType::Pass, gridiron::OutcomeType::Normal, 5, 38);
    gridiron::GameState newState = applyOutcome(curr_state, playOutcome);

    EXPECT_EQ(newState.quarter, 2);
    EXPECT_EQ(newState.time_remaining_seconds, 900);
    EXPECT_EQ(newState.possession, gridiron::Possession::Home);
    EXPECT_EQ(newState.down, 2);
    EXPECT_EQ(newState.distance, 5);
    EXPECT_EQ(newState.ball_position, 30);
    EXPECT_EQ(newState.home_score, 0);
    EXPECT_EQ(newState.away_score, 0);
    EXPECT_FALSE(newState.game_over);
}

TEST(ApplyOutcomeTest, RunWithFirstDownConversionNearGoal) {
    auto curr_state = gridiron::GameState{.ball_position = 85};
    auto playOutcome = gridiron::PlayOutcome(gridiron::PlayType::Run, gridiron::OutcomeType::Normal, 10, 38);
    gridiron::GameState newState = applyOutcome(curr_state, playOutcome);

    EXPECT_EQ(newState.quarter, 1);
    EXPECT_EQ(newState.time_remaining_seconds, 862);
    EXPECT_EQ(newState.possession, gridiron::Possession::Home);
    EXPECT_EQ(newState.down, 1);
    EXPECT_EQ(newState.distance, 5);
    EXPECT_EQ(newState.ball_position, 95);
    EXPECT_EQ(newState.home_score, 0);
    EXPECT_EQ(newState.away_score, 0);
    EXPECT_FALSE(newState.game_over);
}

TEST(ApplyOutcomeTest, TurnoverWithSpotNearGoal) {
    auto curr_state = gridiron::GameState{.ball_position = 50};
    auto playOutcome = gridiron::PlayOutcome(gridiron::PlayType::Run, gridiron::OutcomeType::Turnover, -48, 38);
    gridiron::GameState newState = applyOutcome(curr_state, playOutcome);

    EXPECT_EQ(newState.quarter, 1);
    EXPECT_EQ(newState.time_remaining_seconds, 862);
    EXPECT_EQ(newState.possession, gridiron::Possession::Away);
    EXPECT_EQ(newState.down, 1);
    EXPECT_EQ(newState.distance, 2);
    EXPECT_EQ(newState.ball_position, 98);
    EXPECT_EQ(newState.home_score, 0);
    EXPECT_EQ(newState.away_score, 0);
    EXPECT_FALSE(newState.game_over);
}

TEST(ApplyOutcomeTest, PassWithHalftimeAndHomeOpening) {
    auto curr_state = gridiron::GameState{.quarter = 2, .time_remaining_seconds = 10};
    auto playOutcome = gridiron::PlayOutcome(gridiron::PlayType::Pass, gridiron::OutcomeType::Normal, 5, 38);
    gridiron::GameState newState = applyOutcome(curr_state, playOutcome);

    EXPECT_EQ(newState.quarter, 3);
    EXPECT_EQ(newState.time_remaining_seconds, 900);
    EXPECT_EQ(newState.possession, gridiron::Possession::Away);
    EXPECT_EQ(newState.down, 1);
    EXPECT_EQ(newState.distance, 10);
    EXPECT_EQ(newState.ball_position, 25);
    EXPECT_EQ(newState.home_score, 0);
    EXPECT_EQ(newState.away_score, 0);
    EXPECT_FALSE(newState.game_over);
}

TEST(ApplyOutcomeTest, RunWithHaltimeAndAwayAwayOpening) {
    auto curr_state = gridiron::GameState{.quarter = 2, .time_remaining_seconds = 10, .opening_possession = gridiron::Possession::Away};
    auto playOutcome = gridiron::PlayOutcome(gridiron::PlayType::Run, gridiron::OutcomeType::Normal, 5, 38);
    gridiron::GameState newState = applyOutcome(curr_state, playOutcome);

    EXPECT_EQ(newState.quarter, 3);
    EXPECT_EQ(newState.time_remaining_seconds, 900);
    EXPECT_EQ(newState.possession, gridiron::Possession::Home);
    EXPECT_EQ(newState.down, 1);
    EXPECT_EQ(newState.distance, 10);
    EXPECT_EQ(newState.ball_position, 25);
    EXPECT_EQ(newState.home_score, 0);
    EXPECT_EQ(newState.away_score, 0);
    EXPECT_FALSE(newState.game_over);
}

TEST(ApplyOutcomeTest, RunIn4thQuarterToEndGame) {
    auto curr_state = gridiron::GameState{.quarter = 4, .time_remaining_seconds = 10};
    auto playOutcome = gridiron::PlayOutcome(gridiron::PlayType::Run, gridiron::OutcomeType::Normal, 5, 38);
    gridiron::GameState newState = applyOutcome(curr_state, playOutcome);

    EXPECT_TRUE(newState.game_over);
}

TEST(ApplyOutcomeTest, FieldGoalMadeToEndHalf) {
    auto curr_state = gridiron::GameState{.quarter = 2, .time_remaining_seconds = 10, .possession = gridiron::Possession::Away};
    auto playOutcome = gridiron::PlayOutcome(gridiron::PlayType::FieldGoalAttempt, gridiron::OutcomeType::FieldGoalMade, 0, 38);
    gridiron::GameState newState = applyOutcome(curr_state, playOutcome);

    EXPECT_EQ(newState.quarter, 3);
    EXPECT_EQ(newState.time_remaining_seconds, 900);
    EXPECT_EQ(newState.possession, gridiron::Possession::Home);
    EXPECT_EQ(newState.down, 1);
    EXPECT_EQ(newState.distance, 10);
    EXPECT_EQ(newState.ball_position, 25);
    EXPECT_EQ(newState.home_score, 0);
    EXPECT_EQ(newState.away_score, 3);
    EXPECT_FALSE(newState.game_over);
}

TEST(ApplyOutcomeTest, SafetyToEndHalf) {
    auto curr_state = gridiron::GameState{.quarter = 2, .time_remaining_seconds = 10, .ball_position = 4};
    auto playOutcome = gridiron::PlayOutcome(gridiron::PlayType::Run, gridiron::OutcomeType::Normal, -7, 38);
    gridiron::GameState newState = applyOutcome(curr_state, playOutcome);

    EXPECT_EQ(newState.quarter, 3);
    EXPECT_EQ(newState.time_remaining_seconds, 900);
    EXPECT_EQ(newState.possession, gridiron::Possession::Away);
    EXPECT_EQ(newState.down, 1);
    EXPECT_EQ(newState.distance, 10);
    EXPECT_EQ(newState.ball_position, 25);
    EXPECT_EQ(newState.home_score, 0);
    EXPECT_EQ(newState.away_score, 2);
    EXPECT_FALSE(newState.game_over);
}

TEST(ApplyOutcomeTest, TouchdownToEndGame) {
    auto curr_state = gridiron::GameState{.quarter = 4, .time_remaining_seconds = 10, .opening_possession = gridiron::Possession::Away, .ball_position = 90};
    auto playOutcome = gridiron::PlayOutcome(gridiron::PlayType::Run, gridiron::OutcomeType::Normal, 15, 38);
    gridiron::GameState newState = applyOutcome(curr_state, playOutcome);

    EXPECT_EQ(newState.home_score, 7);
    EXPECT_EQ(newState.away_score, 0);
    EXPECT_TRUE(newState.game_over);
}

TEST(ApplyOutcomeTest, RunReachesExactlyGoalLineScoresTouchdown) {
    auto curr_state = gridiron::GameState{.ball_position = 90};
    auto playOutcome = gridiron::PlayOutcome(gridiron::PlayType::Run, gridiron::OutcomeType::Normal, 10, 38);
    gridiron::GameState newState = applyOutcome(curr_state, playOutcome);

    EXPECT_EQ(newState.quarter, 1);
    EXPECT_EQ(newState.time_remaining_seconds, 862);
    EXPECT_EQ(newState.possession, gridiron::Possession::Away);
    EXPECT_EQ(newState.down, 1);
    EXPECT_EQ(newState.distance, 10);
    EXPECT_EQ(newState.ball_position, 25);
    EXPECT_EQ(newState.home_score, 7);
    EXPECT_EQ(newState.away_score, 0);
    EXPECT_FALSE(newState.game_over);
}

TEST(ApplyOutcomeTest, RunCrossesBeyondGoalLineScoresTouchdown) {
    auto curr_state = gridiron::GameState{.ball_position = 90};
    auto playOutcome = gridiron::PlayOutcome(gridiron::PlayType::Run, gridiron::OutcomeType::Normal, 15, 38);
    gridiron::GameState newState = applyOutcome(curr_state, playOutcome);

    EXPECT_EQ(newState.quarter, 1);
    EXPECT_EQ(newState.time_remaining_seconds, 862);
    EXPECT_EQ(newState.possession, gridiron::Possession::Away);
    EXPECT_EQ(newState.down, 1);
    EXPECT_EQ(newState.distance, 10);
    EXPECT_EQ(newState.ball_position, 25);
    EXPECT_EQ(newState.home_score, 7);
    EXPECT_EQ(newState.away_score, 0);
    EXPECT_FALSE(newState.game_over);
}

TEST(ApplyOutcomeTest, PassCrossesGoalLineOnFinalPlayOfHalfStillScores) {
    auto curr_state = gridiron::GameState{.quarter = 2, .time_remaining_seconds = 10, .opening_possession = gridiron::Possession::Away, .ball_position = 92};
    auto playOutcome = gridiron::PlayOutcome(gridiron::PlayType::Pass, gridiron::OutcomeType::Normal, 10, 38);
    gridiron::GameState newState = applyOutcome(curr_state, playOutcome);

    EXPECT_EQ(newState.quarter, 3);
    EXPECT_EQ(newState.time_remaining_seconds, 900);
    // Scoring play's own kickoff assignment (opponent of scorer receives)
    // must win over the halftime "opponent of opening possession" rule.
    EXPECT_EQ(newState.possession, gridiron::Possession::Away);
    EXPECT_EQ(newState.down, 1);
    EXPECT_EQ(newState.distance, 10);
    EXPECT_EQ(newState.ball_position, 25);
    EXPECT_EQ(newState.home_score, 7);
    EXPECT_EQ(newState.away_score, 0);
    EXPECT_FALSE(newState.game_over);
}

TEST(ApplyOutcomeTest, RunGainsExactYardsNeededForFirstDown) {
    auto curr_state = gridiron::GameState();
    auto playOutcome = gridiron::PlayOutcome(gridiron::PlayType::Run, gridiron::OutcomeType::Normal, 10, 38);
    gridiron::GameState newState = applyOutcome(curr_state, playOutcome);

    EXPECT_EQ(newState.quarter, 1);
    EXPECT_EQ(newState.time_remaining_seconds, 862);
    EXPECT_EQ(newState.possession, gridiron::Possession::Home);
    EXPECT_EQ(newState.down, 1);
    EXPECT_EQ(newState.distance, 10);
    EXPECT_EQ(newState.ball_position, 35);
    EXPECT_EQ(newState.home_score, 0);
    EXPECT_EQ(newState.away_score, 0);
    EXPECT_FALSE(newState.game_over);
}

TEST(ApplyOutcomeTest, TurnoverAtMidfield) {
    auto curr_state = gridiron::GameState{.ball_position = 50};
    auto playOutcome = gridiron::PlayOutcome(gridiron::PlayType::Run, gridiron::OutcomeType::Turnover, 0, 38);
    gridiron::GameState newState = applyOutcome(curr_state, playOutcome);

    EXPECT_EQ(newState.quarter, 1);
    EXPECT_EQ(newState.time_remaining_seconds, 862);
    EXPECT_EQ(newState.possession, gridiron::Possession::Away);
    EXPECT_EQ(newState.down, 1);
    EXPECT_EQ(newState.distance, 10);
    EXPECT_EQ(newState.ball_position, 50);
    EXPECT_EQ(newState.home_score, 0);
    EXPECT_EQ(newState.away_score, 0);
    EXPECT_FALSE(newState.game_over);
}

TEST(ApplyOutcomeTest, FailedFourthDownAtMidfield) {
    auto curr_state = gridiron::GameState{.down = 4, .ball_position = 50};
    auto playOutcome = gridiron::PlayOutcome(gridiron::PlayType::Pass, gridiron::OutcomeType::Normal, 3, 38);
    gridiron::GameState newState = applyOutcome(curr_state, playOutcome);

    EXPECT_EQ(newState.quarter, 1);
    EXPECT_EQ(newState.time_remaining_seconds, 862);
    EXPECT_EQ(newState.possession, gridiron::Possession::Away);
    EXPECT_EQ(newState.down, 1);
    EXPECT_EQ(newState.distance, 10);
    EXPECT_EQ(newState.ball_position, 47);
    EXPECT_EQ(newState.home_score, 0);
    EXPECT_EQ(newState.away_score, 0);
    EXPECT_FALSE(newState.game_over);
}

TEST(ApplyOutcomeTest, Q4EndsInTie) {
    auto curr_state = gridiron::GameState{.quarter = 4, .time_remaining_seconds = 10, .home_score = 21, .away_score = 21};
    auto playOutcome = gridiron::PlayOutcome(gridiron::PlayType::Run, gridiron::OutcomeType::Normal, 2, 38);
    gridiron::GameState newState = applyOutcome(curr_state, playOutcome);

    EXPECT_EQ(newState.quarter, 4);
    EXPECT_TRUE(newState.game_over);
    EXPECT_EQ(newState.home_score, 21);
    EXPECT_EQ(newState.away_score, 21);
}

// Bounds bugs: the Turnover case applies yards_gained and flips the field
// without any 0-100 guard, unlike the Normal case. Both tests assert the
// legal-state invariant (acceptance criterion #3) rather than a specific
// clamped value, so they stay valid whichever fix we pick
TEST(ApplyOutcomeTest, TurnoverNearOpponentGoalLineStaysOnField) {
    // 90 + 15 = 105, flipped to 100 - 105 = -5 before any guard runs.
    auto curr_state = gridiron::GameState{.ball_position = 90};
    auto playOutcome = gridiron::PlayOutcome(gridiron::PlayType::Pass, gridiron::OutcomeType::Turnover, 15, 38);
    gridiron::GameState newState = applyOutcome(curr_state, playOutcome);

    EXPECT_GE(newState.ball_position, 0);
    EXPECT_LE(newState.ball_position, 100);
    EXPECT_EQ(newState.possession, gridiron::Possession::Away);
    EXPECT_EQ(newState.down, 1);
    EXPECT_GE(newState.distance, 0);
}

TEST(ApplyOutcomeTest, TurnoverBehindOwnGoalLineStaysOnField) {
    // 5 - 10 = -5, flipped to 100 - (-5) = 105 before any guard runs.
    auto curr_state = gridiron::GameState{.ball_position = 5};
    auto playOutcome = gridiron::PlayOutcome(gridiron::PlayType::Run, gridiron::OutcomeType::Turnover, -10, 38);
    gridiron::GameState newState = applyOutcome(curr_state, playOutcome);

    EXPECT_GE(newState.ball_position, 0);
    EXPECT_LE(newState.ball_position, 100);
    EXPECT_EQ(newState.possession, gridiron::Possession::Away);
    EXPECT_EQ(newState.down, 1);
    EXPECT_GE(newState.distance, 0);
}

TEST(ApplyOutcomeTest, MissedFieldGoalIgnoresYardsGained) {
    // A missed field goal has no return; ball_position reflects only the
    // spot of the kick (100 - spot), regardless of the yards_gained field.
    auto curr_state = gridiron::GameState{.ball_position = 25};
    auto playOutcome = gridiron::PlayOutcome(gridiron::PlayType::FieldGoalAttempt, gridiron::OutcomeType::Turnover, 9999, 38);
    gridiron::GameState newState = applyOutcome(curr_state, playOutcome);

    EXPECT_EQ(newState.possession, gridiron::Possession::Away);
    EXPECT_EQ(newState.ball_position, 75);
    EXPECT_EQ(newState.down, 1);
    EXPECT_EQ(newState.distance, 10);
}
