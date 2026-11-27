#include <gtest/gtest.h>
#include <random>

#include "../src/engine/AggressivePolicy.h"
#include "../src/engine/ConservativePolicy.h"
#include "../src/engine/LateGamePolicy.h"

namespace {
// Home trails 24-27 with 5:00 left in Q4, 4th and 4 at the opponent's 30.
gridiron::GameState lateFourthDown() {
    return gridiron::GameState{.quarter = 4,
                               .time_remaining_seconds = 300,
                               .possession = gridiron::Possession::Home,
                               .down = 4,
                               .distance = 4,
                               .ball_position = 70,
                               .home_score = 24,
                               .away_score = 27};
}

bool goesForIt(gridiron::PlayType play) {
    return play == gridiron::PlayType::Run || play == gridiron::PlayType::Pass;
}
} // namespace

class PolicyVariantsTest : public ::testing::Test {
protected:
    const gridiron::Matchup teams{};
    std::mt19937 rng{42};
    gridiron::RandomSource random{rng};
};

// --- AggressivePolicy -------------------------------------------------------

TEST_F(PolicyVariantsTest, AggressiveGoesForItOnShortLateFourthDowns) {
    const gridiron::AggressivePolicy policy;
    gridiron::GameState state = lateFourthDown();
    EXPECT_TRUE(goesForIt(policy.choosePlay(state, teams, random)));

    state.home_score = 27; // tied still counts as not leading
    EXPECT_TRUE(goesForIt(policy.choosePlay(state, teams, random)));
}

// One step past each edge falls back to LateGamePolicy, which kicks the field goal here.
TEST_F(PolicyVariantsTest, AggressiveFallsBackOutsideItsWindow) {
    const gridiron::AggressivePolicy policy;

    gridiron::GameState early = lateFourthDown();
    early.time_remaining_seconds = 301;
    EXPECT_EQ(policy.choosePlay(early, teams, random), gridiron::PlayType::FieldGoalAttempt);

    gridiron::GameState longer = lateFourthDown();
    longer.distance = 5;
    EXPECT_EQ(policy.choosePlay(longer, teams, random), gridiron::PlayType::FieldGoalAttempt);

    gridiron::GameState leading = lateFourthDown();
    leading.home_score = 28;
    EXPECT_EQ(policy.choosePlay(leading, teams, random), gridiron::PlayType::FieldGoalAttempt);

    gridiron::GameState thirdQuarter = lateFourthDown();
    thirdQuarter.quarter = 3;
    EXPECT_EQ(policy.choosePlay(thirdQuarter, teams, random), gridiron::PlayType::FieldGoalAttempt);
}

// --- ConservativePolicy -----------------------------------------------------

TEST_F(PolicyVariantsTest, ConservativeKicksAtTheFieldGoalRangeEdge) {
    const gridiron::ConservativePolicy policy;
    gridiron::GameState state = lateFourthDown();

    state.ball_position = 60; // 40 yards out
    EXPECT_EQ(policy.choosePlay(state, teams, random), gridiron::PlayType::FieldGoalAttempt);
    state.ball_position = 59; // 41 yards out
    EXPECT_EQ(policy.choosePlay(state, teams, random), gridiron::PlayType::Punt);
}

// Down 7 with 0:40 left and 4th and 1 at its own 30: LateGamePolicy goes for it, the conservative policy punts.
TEST_F(PolicyVariantsTest, ConservativeNeverGoesForItOnFourthDown) {
    const gridiron::ConservativePolicy policy;
    gridiron::GameState state = lateFourthDown();
    state.time_remaining_seconds = 40;
    state.distance = 1;
    state.ball_position = 30;
    state.home_score = 20;
    EXPECT_EQ(policy.choosePlay(state, teams, random), gridiron::PlayType::Punt);

    state.down = 3;
    EXPECT_TRUE(goesForIt(policy.choosePlay(state, teams, random)));
}

// --- Shared timeouts --------------------------------------------------------

TEST_F(PolicyVariantsTest, VariantsUseLateGameTimeouts) {
    const gridiron::LateGamePolicy lateGame;
    const gridiron::ConservativePolicy conservative;
    const gridiron::AggressivePolicy aggressive;
    gridiron::GameState state = lateFourthDown();
    gridiron::PlayOutcome play{};
    play.elapsed_seconds = 6;

    for (int seconds : {6, 7, 66, 67, 126, 127}) {
        state.time_remaining_seconds = seconds;
        for (gridiron::Possession team : {gridiron::Possession::Home, gridiron::Possession::Away}) {
            const bool expected = lateGame.callTimeout(state, play, team);
            EXPECT_EQ(conservative.callTimeout(state, play, team), expected) << seconds;
            EXPECT_EQ(aggressive.callTimeout(state, play, team), expected) << seconds;
        }
    }
}
