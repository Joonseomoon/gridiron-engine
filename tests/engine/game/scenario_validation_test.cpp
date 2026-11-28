#include <functional>
#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "engine/core/GameState.h"
#include "engine/game/ScenarioValidation.h"

namespace {
// The M4.md example: BAL (home) trails SEA 24-27, 3:14 left, 4th and 2 at SEA's 43.
gridiron::GameState exampleScenario() {
    return gridiron::GameState{.quarter = 4,
                               .time_remaining_seconds = 194,
                               .possession = gridiron::Possession::Home,
                               .down = 4,
                               .distance = 2,
                               .ball_position = 57,
                               .home_score = 24,
                               .away_score = 27};
}

// Checks one field at both edges of its range and one step outside each.
void expectRange(const std::function<void(gridiron::GameState&, int)>& setField, int low, int high,
                 gridiron::GameState base = exampleScenario()) {
    for (int value : {low, high}) {
        gridiron::GameState state = base;
        setField(state, value);
        EXPECT_TRUE(gridiron::validateScenario(state).empty()) << "value " << value << " should be valid";
    }
    for (int value : {low - 1, high + 1}) {
        gridiron::GameState state = base;
        setField(state, value);
        EXPECT_EQ(gridiron::validateScenario(state).size(), 1u) << "value " << value << " should be invalid";
    }
}
} // namespace

TEST(ScenarioValidationTest, ExampleScenarioIsValid) {
    EXPECT_TRUE(gridiron::validateScenario(exampleScenario()).empty());
}

TEST(ScenarioValidationTest, OpeningKickoffStateIsValid) {
    EXPECT_TRUE(gridiron::validateScenario(gridiron::GameState{}).empty());
}

TEST(ScenarioValidationTest, QuarterIsRegulationOnly) {
    expectRange([](gridiron::GameState& s, int v) { s.quarter = v; }, 1, 4);
}

TEST(ScenarioValidationTest, TimeRemainingRange) {
    expectRange([](gridiron::GameState& s, int v) { s.time_remaining_seconds = v; }, 1, 900);
}

TEST(ScenarioValidationTest, DownRange) {
    expectRange([](gridiron::GameState& s, int v) { s.down = v; }, 1, 4);
}

TEST(ScenarioValidationTest, BallPositionRange) {
    // Distance 1 fits even at the 99, so only the ball position is being tested.
    gridiron::GameState base = exampleScenario();
    base.distance = 1;
    expectRange([](gridiron::GameState& s, int v) { s.ball_position = v; }, 1, 99, base);
}

TEST(ScenarioValidationTest, DistanceRangeUpToTheGoalLine) {
    // Ball at 57, so 43 yards to the goal.
    expectRange([](gridiron::GameState& s, int v) { s.distance = v; }, 1, 43);
}

TEST(ScenarioValidationTest, BadBallPositionDoesNotAlsoReportDistance) {
    gridiron::GameState state = exampleScenario();
    state.ball_position = 120;
    state.distance = 10;
    EXPECT_EQ(gridiron::validateScenario(state).size(), 1u);
}

TEST(ScenarioValidationTest, TimeoutRanges) {
    expectRange([](gridiron::GameState& s, int v) { s.home_timeouts = v; }, 0, 3);
    expectRange([](gridiron::GameState& s, int v) { s.away_timeouts = v; }, 0, 3);
}

TEST(ScenarioValidationTest, ScoresMustNotBeNegative) {
    gridiron::GameState state = exampleScenario();
    state.home_score = 0;
    state.away_score = 0;
    EXPECT_TRUE(gridiron::validateScenario(state).empty());

    state.home_score = -1;
    EXPECT_EQ(gridiron::validateScenario(state).size(), 1u);
    state.away_score = -1;
    EXPECT_EQ(gridiron::validateScenario(state).size(), 2u);
}

TEST(ScenarioValidationTest, FinishedOrOvertimeStatesAreInvalid) {
    gridiron::GameState over = exampleScenario();
    over.game_over = true;
    EXPECT_EQ(gridiron::validateScenario(over).size(), 1u);

    gridiron::GameState overtime = exampleScenario();
    overtime.overtime_possessions_completed = 1;
    EXPECT_EQ(gridiron::validateScenario(overtime).size(), 1u);
}

TEST(ScenarioValidationTest, ReportsEveryProblemTogether) {
    gridiron::GameState state = exampleScenario();
    state.quarter = 5;
    state.down = 0;
    state.distance = 0;
    state.home_timeouts = 4;
    state.away_score = -3;

    const std::vector<std::string> problems = gridiron::validateScenario(state);
    EXPECT_EQ(problems.size(), 5u);
}

TEST(ScenarioValidationTest, MessageNamesTheFieldAndValue) {
    gridiron::GameState state = exampleScenario();
    state.down = 5;
    const std::vector<std::string> problems = gridiron::validateScenario(state);
    ASSERT_EQ(problems.size(), 1u);
    EXPECT_NE(problems[0].find("down"), std::string::npos);
    EXPECT_NE(problems[0].find("5"), std::string::npos);
}
