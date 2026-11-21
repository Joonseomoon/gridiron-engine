#include <gtest/gtest.h>

#include <cmath>
#include <vector>

#include "../src/engine/DefaultPolicy.h"
#include "../src/engine/ModelConstants.h"
#include "../src/engine/OutcomeSampler.h"
#include "../src/engine/RandomSource.h"

namespace {

// Large enough that the standard error of a sampled mean is small relative to
// the tolerances below, so the bands catch real changes without ever flaking.
// Every loop uses a fixed seed, so these tests are fully deterministic.
constexpr int kSamples = 10000;
constexpr unsigned kSeed = 42;

double mean(const std::vector<int>& values) {
    double total = 0.0;
    for (const int value : values) {
        total += value;
    }
    return total / static_cast<double>(values.size());
}

double stddev(const std::vector<int>& values) {
    const double m = mean(values);
    double sumSquares = 0.0;
    for (const int value : values) {
        sumSquares += (value - m) * (value - m);
    }
    return std::sqrt(sumSquares / static_cast<double>(values.size()));
}

} // namespace

class OutcomeSamplerTest : public ::testing::Test {
protected:
    gridiron::TeamProfile offense{"Offense", .8, .7, .8, .7, .01, .8};

    gridiron::TeamProfile defense{"Defense", .7, .9, .9, .5, .02, .8};
};

TEST_F(OutcomeSamplerTest, RunSamplerReturnsRun) {
    std::mt19937 rng(42);
    gridiron::RandomSource random{rng};
    gridiron::PlayOutcome outcome = gridiron::sampleRunOutcome(offense, defense, random);
    EXPECT_EQ(outcome.play_type, gridiron::PlayType::Run);
}

TEST_F(OutcomeSamplerTest, PassSamplerReturnsPass) {
    std::mt19937 rng(42);
    gridiron::RandomSource random{rng};
    gridiron::PlayOutcome outcome = gridiron::samplePassOutcome(offense, defense, random);
    EXPECT_EQ(outcome.play_type, gridiron::PlayType::Pass);
}

TEST_F(OutcomeSamplerTest, RunTurnoverRateOneAlwaysTurnover) {
    std::mt19937 rng(42);
    gridiron::RandomSource random{rng};
    offense.turnover_rate = 1.0;
    gridiron::PlayOutcome outcome = gridiron::sampleRunOutcome(offense, defense, random);
    EXPECT_EQ(outcome.outcome_type, gridiron::OutcomeType::Turnover);
}

TEST_F(OutcomeSamplerTest, RunTurnoverRateZeroNeverTurnover) {
    std::mt19937 rng(42);
    gridiron::RandomSource random{rng};
    offense.turnover_rate = 0.0;
    gridiron::PlayOutcome outcome = gridiron::sampleRunOutcome(offense, defense, random);
    EXPECT_EQ(outcome.outcome_type, gridiron::OutcomeType::Normal);
}

TEST_F(OutcomeSamplerTest, PassTurnoverRateOneAlwaysTurnover) {
    std::mt19937 rng(42);
    gridiron::RandomSource random{rng};
    offense.turnover_rate = 1.0;
    gridiron::PlayOutcome outcome = gridiron::samplePassOutcome(offense, defense, random);
    EXPECT_EQ(outcome.outcome_type, gridiron::OutcomeType::Turnover);
}

TEST_F(OutcomeSamplerTest, PassTurnoverRateZeroNeverTurnover) {
    std::mt19937 rng(42);
    gridiron::RandomSource random{rng};
    offense.turnover_rate = 0.0;
    gridiron::PlayOutcome outcome = gridiron::samplePassOutcome(offense, defense, random);
    EXPECT_NE(outcome.outcome_type, gridiron::OutcomeType::Turnover);
}

TEST_F(OutcomeSamplerTest, DefaultPolicyReturnsFieldGoalAttempt) {
    std::mt19937 rng(42);
    gridiron::RandomSource random{rng};
    const gridiron::DefaultPolicy policy;
    const gridiron::Matchup matchup{offense, defense};
    auto gameState = gridiron::GameState{.down = 4, .ball_position = 75};
    gridiron::PlayType playType = policy.choosePlay(gameState, matchup, random);
    EXPECT_EQ(playType, gridiron::PlayType::FieldGoalAttempt);
}

TEST_F(OutcomeSamplerTest, DefaultPolicyAttemptsFieldGoalAtExactly40) {
    std::mt19937 rng(42);
    gridiron::RandomSource random{rng};
    const gridiron::DefaultPolicy policy;
    const gridiron::Matchup matchup{offense, defense};
    auto gameState = gridiron::GameState{.down = 4, .ball_position = 60};
    gridiron::PlayType playType = policy.choosePlay(gameState, matchup, random);
    EXPECT_EQ(playType, gridiron::PlayType::FieldGoalAttempt);
}

TEST_F(OutcomeSamplerTest, DefaultPolicyPuntsOnLongFourthDownWithoutDrawingRandomness) {
    std::mt19937 rng(42);
    const auto original = rng;
    gridiron::RandomSource random{rng};
    const gridiron::DefaultPolicy policy;
    const gridiron::Matchup matchup{offense, defense};
    auto gameState = gridiron::GameState{.down = 4, .ball_position = 59};
    gridiron::PlayType playType = policy.choosePlay(gameState, matchup, random);
    EXPECT_EQ(playType, gridiron::PlayType::Punt);
    EXPECT_EQ(rng, original);
}

TEST_F(OutcomeSamplerTest, DefaultPolicyFourthDownShortYardageBoundary) {
    const gridiron::DefaultPolicy policy;
    const gridiron::Matchup matchup{offense, defense};
    for (int distance : {1, 2, 3}) {
        SCOPED_TRACE(distance);
        std::mt19937 rng(42);
        gridiron::RandomSource random{rng};
        const gridiron::GameState state{.down = 4, .distance = distance, .ball_position = 59};
        const auto play = policy.choosePlay(state, matchup, random);
        if (distance <= 2) {
            EXPECT_TRUE(play == gridiron::PlayType::Run || play == gridiron::PlayType::Pass);
        } else {
            EXPECT_EQ(play, gridiron::PlayType::Punt);
        }
    }
}

TEST_F(OutcomeSamplerTest, DefaultPolicyReturnsRunOrPassInFieldGoalRangeBeforeFourthDown) {
    std::mt19937 rng(42);
    gridiron::RandomSource random{rng};
    const gridiron::DefaultPolicy policy;
    const gridiron::Matchup matchup{offense, defense};
    auto gameState = gridiron::GameState{.down = 3, .ball_position = 75};
    gridiron::PlayType playType = policy.choosePlay(gameState, matchup, random);
    EXPECT_TRUE(playType == gridiron::PlayType::Run || playType == gridiron::PlayType::Pass);
}

TEST_F(OutcomeSamplerTest, SamplePlayOutcomeDispatchesFieldGoalAttempt) {
    gridiron::Matchup matchup{.home = offense, .away = defense};
    std::mt19937 rng(42);
    gridiron::RandomSource random{rng};
    auto gameState = gridiron::GameState{.down = 4, .ball_position = 75};
    const gridiron::DefaultPolicy policy;
    const gridiron::PlayType playType = policy.choosePlay(gameState, matchup, random);
    gridiron::PlayOutcome outcome = gridiron::samplePlayOutcome(gameState, matchup, playType, random);
    EXPECT_EQ(outcome.play_type, gridiron::PlayType::FieldGoalAttempt);
}

// --- Yardage formulas -------------------------------------------------------
// Single-sample tests cannot pin these down at all: they verify the base yards,
// the rating multipliers, which ratings feed which formula, the sign of the
// rating difference, the spread, and the rounding.

TEST_F(OutcomeSamplerTest, RunYardageMatchesExpectedMeanAndSpread) {
    // expected = 4.3 + 6 * (0.7 run offense - 0.5 run defense) = 5.5, sd 3.0
    std::mt19937 rng(kSeed);
    gridiron::RandomSource random{rng};
    std::vector<int> yards;
    yards.reserve(kSamples);
    for (int i = 0; i < kSamples; ++i) {
        yards.push_back(gridiron::sampleRunOutcome(offense, defense, random).yards_gained);
    }
    EXPECT_NEAR(mean(yards), 5.5, 0.2);
    EXPECT_NEAR(stddev(yards), 3.0, 0.2);
}

TEST_F(OutcomeSamplerTest, PassYardageMatchesExpectedMeanAndSpread) {
    // expected = 10.5 + 10 * (0.8 pass offense - 0.9 pass defense) = 9.5, sd 8.0, for completions only
    std::mt19937 rng(kSeed);
    gridiron::RandomSource random{rng};
    std::vector<int> yards;
    yards.reserve(kSamples);
    for (int i = 0; i < kSamples; ++i) {
        const auto outcome = gridiron::samplePassOutcome(offense, defense, random);
        if (outcome.outcome_type == gridiron::OutcomeType::Normal) {
            yards.push_back(outcome.yards_gained);
        }
    }
    EXPECT_NEAR(mean(yards), 9.5, 0.3);
    EXPECT_NEAR(stddev(yards), 8.0, 0.3);
}

// Catches a flipped rating difference, and catches the run formula reading pass
// ratings: both profiles below leave the pass ratings at zero, so a run formula
// using them would produce identical means.
TEST_F(OutcomeSamplerTest, BetterRunOffenseGainsMoreOnAverage) {
    gridiron::TeamProfile strong{.name = "Strong", .offensive_run_rating = 0.9};
    gridiron::TeamProfile weak{.name = "Weak", .offensive_run_rating = 0.3};

    std::mt19937 rng(kSeed);

    gridiron::RandomSource random{rng};
    std::vector<int> strongYards;
    std::vector<int> weakYards;
    strongYards.reserve(kSamples);
    weakYards.reserve(kSamples);
    for (int i = 0; i < kSamples; ++i) {
        strongYards.push_back(gridiron::sampleRunOutcome(strong, defense, random).yards_gained);
        weakYards.push_back(gridiron::sampleRunOutcome(weak, defense, random).yards_gained);
    }
    EXPECT_GT(mean(strongYards), mean(weakYards));
}

TEST_F(OutcomeSamplerTest, BetterPassOffenseGainsMoreOnAverage) {
    gridiron::TeamProfile strong{.name = "Strong", .offensive_pass_rating = 0.9};
    gridiron::TeamProfile weak{.name = "Weak", .offensive_pass_rating = 0.3};

    std::mt19937 rng(kSeed);

    gridiron::RandomSource random{rng};
    std::vector<int> strongYards;
    std::vector<int> weakYards;
    strongYards.reserve(kSamples);
    weakYards.reserve(kSamples);
    for (int i = 0; i < kSamples; ++i) {
        strongYards.push_back(gridiron::samplePassOutcome(strong, defense, random).yards_gained);
        weakYards.push_back(gridiron::samplePassOutcome(weak, defense, random).yards_gained);
    }
    EXPECT_GT(mean(strongYards), mean(weakYards));
}

// --- Turnovers --------------------------------------------------------------
// Rates of exactly 0 and 1 catch a wrong variable but not a wrong threshold.

TEST_F(OutcomeSamplerTest, RunTurnoverFrequencyMatchesRate) {
    gridiron::TeamProfile fumbler = offense;
    fumbler.turnover_rate = 0.3;

    std::mt19937 rng(kSeed);

    gridiron::RandomSource random{rng};
    int turnovers = 0;
    for (int i = 0; i < kSamples; ++i) {
        if (gridiron::sampleRunOutcome(fumbler, defense, random).outcome_type == gridiron::OutcomeType::Turnover) {
            ++turnovers;
        }
    }
    EXPECT_NEAR(turnovers / static_cast<double>(kSamples), 0.3, 0.02);
}

TEST_F(OutcomeSamplerTest, PassTurnoverFrequencyMatchesRate) {
    gridiron::TeamProfile fumbler = offense;
    fumbler.turnover_rate = 0.3;

    std::mt19937 rng(kSeed);

    gridiron::RandomSource random{rng};
    int turnovers = 0;
    for (int i = 0; i < kSamples; ++i) {
        if (gridiron::samplePassOutcome(fumbler, defense, random).outcome_type == gridiron::OutcomeType::Turnover) {
            ++turnovers;
        }
    }
    EXPECT_NEAR(turnovers / static_cast<double>(kSamples), 0.3, 0.02);
}

TEST_F(OutcomeSamplerTest, RunTurnoverStillReportsRunPlayType) {
    std::mt19937 rng(kSeed);
    gridiron::RandomSource random{rng};
    offense.turnover_rate = 1.0;
    const gridiron::PlayOutcome outcome = gridiron::sampleRunOutcome(offense, defense, random);
    EXPECT_EQ(outcome.outcome_type, gridiron::OutcomeType::Turnover);
    EXPECT_EQ(outcome.play_type, gridiron::PlayType::Run);
}

TEST_F(OutcomeSamplerTest, PassTurnoverStillReportsPassPlayType) {
    std::mt19937 rng(kSeed);
    gridiron::RandomSource random{rng};
    offense.turnover_rate = 1.0;
    const gridiron::PlayOutcome outcome = gridiron::samplePassOutcome(offense, defense, random);
    EXPECT_EQ(outcome.outcome_type, gridiron::OutcomeType::Turnover);
    EXPECT_EQ(outcome.play_type, gridiron::PlayType::Pass);
}

// --- Play duration ----------------------------------------------------------
// The floor holds ~98% of the time without its clamp, so one sample cannot tell
// the clamp apart from its absence. The mean checks catch a shifted distribution
// that the floor alone would hide.

TEST_F(OutcomeSamplerTest, NormalPlayTimeRespectsFloorAndMean) {
    // Fixture run yardage averages 5.5, so the over-20-yard branch never fires.
    std::mt19937 rng(kSeed);
    gridiron::RandomSource random{rng};
    std::vector<int> times;
    times.reserve(kSamples);
    for (int i = 0; i < kSamples; ++i) {
        const auto outcome = gridiron::sampleRunOutcome(offense, defense, random);
        times.push_back(outcome.elapsed_seconds + outcome.runoff_seconds);
    }
    for (const int time : times) {
        ASSERT_GE(time, 10);
    }
    EXPECT_NEAR(mean(times), 33.0, 0.5);
}

TEST_F(OutcomeSamplerTest, BigPlayTimeUsesLongerDistribution) {
    // expected = 4.3 + 6 * (10.0 - 0.0) = 64.3 yards, so essentially every play
    // takes the over-20-yard branch, which the fixture profiles never reach.
    gridiron::TeamProfile explosive{.name = "Explosive", .offensive_run_rating = 10.0};
    gridiron::TeamProfile porous{.name = "Porous"};

    std::mt19937 rng(kSeed);

    gridiron::RandomSource random{rng};
    std::vector<int> times;
    times.reserve(kSamples);
    for (int i = 0; i < kSamples; ++i) {
        const gridiron::PlayOutcome outcome = gridiron::sampleRunOutcome(explosive, porous, random);
        ASSERT_GT(outcome.yards_gained, 20);
        times.push_back(outcome.elapsed_seconds + outcome.runoff_seconds);
    }
    for (const int time : times) {
        ASSERT_GE(time, 15);
    }
    EXPECT_NEAR(mean(times), 43.0, 0.5);
}

// --- Play type selection and dispatch ---------------------------------------

TEST_F(OutcomeSamplerTest, DefaultPolicySplitsRunAndPassRoughlyEvenly) {
    std::mt19937 rng(kSeed);
    gridiron::RandomSource random{rng};
    gridiron::GameState gameState;
    const gridiron::DefaultPolicy policy;
    const gridiron::Matchup matchup{offense, defense};
    int runs = 0;
    for (int i = 0; i < kSamples; ++i) {
        if (policy.choosePlay(gameState, matchup, random) == gridiron::PlayType::Run) {
            ++runs;
        }
    }
    EXPECT_NEAR(runs / static_cast<double>(kSamples), 0.5, 0.03);
}

TEST_F(OutcomeSamplerTest, SamplePlayOutcomeDispatchesToTheChosenPlayType) {
    gridiron::Matchup matchup{.home = offense, .away = defense};
    const gridiron::DefaultPolicy policy;
    gridiron::GameState gameState{.down = 4, .ball_position = 75};
    for (gridiron::PlayType type : {gridiron::PlayType::Run, gridiron::PlayType::Pass,
                                    gridiron::PlayType::FieldGoalAttempt, gridiron::PlayType::Punt}) {
        std::mt19937 rng(kSeed);
        gridiron::RandomSource random{rng};
        gridiron::PlayOutcome outcome = gridiron::samplePlayOutcome(gameState, matchup, type, random);
        EXPECT_EQ(outcome.play_type, type);
    }
}

// --- Field goals ------------------------------------------------------------
// The clamp to [0.01, 0.99] means no rating guarantees a make or a miss, so the
// extremes have to be checked in aggregate rather than on a single draw.

TEST_F(OutcomeSamplerTest, FieldGoalMakeRateMatchesFormula) {
    // 0.95 rating, 23 to the goal -> 40 yard kick -> 0.95 - 0.01 * 20 = 0.75
    gridiron::TeamProfile kicker{.name = "Kicker", .field_goal_rating = 0.95};

    std::mt19937 rng(kSeed);

    gridiron::RandomSource random{rng};
    int makes = 0;
    for (int i = 0; i < kSamples; ++i) {
        if (gridiron::sampleFieldGoalOutcome(kicker, 23, random).outcome_type == gridiron::OutcomeType::FieldGoalMade) {
            ++makes;
        }
    }
    EXPECT_NEAR(makes / static_cast<double>(kSamples), 0.75, 0.02);
}

TEST_F(OutcomeSamplerTest, FieldGoalsConvertLessOftenFromLongerRange) {
    gridiron::TeamProfile kicker{.name = "Kicker", .field_goal_rating = 0.95};

    std::mt19937 rng(kSeed);

    gridiron::RandomSource random{rng};
    int shortMakes = 0;
    int longMakes = 0;
    for (int i = 0; i < kSamples; ++i) {
        if (gridiron::sampleFieldGoalOutcome(kicker, 3, random).outcome_type == gridiron::OutcomeType::FieldGoalMade) {
            ++shortMakes;
        }
        if (gridiron::sampleFieldGoalOutcome(kicker, 40, random).outcome_type == gridiron::OutcomeType::FieldGoalMade) {
            ++longMakes;
        }
    }
    EXPECT_GT(shortMakes, longMakes);
}

TEST_F(OutcomeSamplerTest, BetterKickerConvertsMoreFromTheSameSpot) {
    gridiron::TeamProfile good{.name = "Good", .field_goal_rating = 0.95};
    gridiron::TeamProfile poor{.name = "Poor", .field_goal_rating = 0.60};

    std::mt19937 rng(kSeed);

    gridiron::RandomSource random{rng};
    int goodMakes = 0;
    int poorMakes = 0;
    for (int i = 0; i < kSamples; ++i) {
        if (gridiron::sampleFieldGoalOutcome(good, 23, random).outcome_type == gridiron::OutcomeType::FieldGoalMade) {
            ++goodMakes;
        }
        if (gridiron::sampleFieldGoalOutcome(poor, 23, random).outcome_type == gridiron::OutcomeType::FieldGoalMade) {
            ++poorMakes;
        }
    }
    EXPECT_GT(goodMakes, poorMakes);
}

// A miss is reported as a Turnover, so the play type is the only thing telling
// applyOutcome that no return yardage applies.
TEST_F(OutcomeSamplerTest, FieldGoalAlwaysReportsFieldGoalAttemptPlayType) {
    gridiron::TeamProfile kicker{.name = "Kicker", .field_goal_rating = 0.60};

    std::mt19937 rng(kSeed);

    gridiron::RandomSource random{rng};
    int makes = 0;
    int misses = 0;
    for (int i = 0; i < kSamples; ++i) {
        const gridiron::PlayOutcome outcome = gridiron::sampleFieldGoalOutcome(kicker, 23, random);
        ASSERT_EQ(outcome.play_type, gridiron::PlayType::FieldGoalAttempt);
        if (outcome.outcome_type == gridiron::OutcomeType::FieldGoalMade) {
            ++makes;
        } else if (outcome.outcome_type == gridiron::OutcomeType::Turnover) {
            ++misses;
        } else {
            FAIL() << "field goal produced an unexpected outcome type";
        }
    }
    EXPECT_GT(makes, 0);
    EXPECT_GT(misses, 0);
}

TEST_F(OutcomeSamplerTest, FieldGoalClockRespectsFloorAndMean) {
    gridiron::TeamProfile kicker{.name = "Kicker", .field_goal_rating = 0.95};

    std::mt19937 rng(kSeed);

    gridiron::RandomSource random{rng};
    std::vector<int> times;
    times.reserve(kSamples);
    for (int i = 0; i < kSamples; ++i) {
        const auto outcome = gridiron::sampleFieldGoalOutcome(kicker, 23, random);
        times.push_back(outcome.elapsed_seconds + outcome.runoff_seconds);
    }
    for (const int time : times) {
        ASSERT_GE(time, 10);
    }
    EXPECT_NEAR(mean(times), 33.0, 0.5);
}

TEST_F(OutcomeSamplerTest, PuntAlwaysReportsPuntWithFixedPlayTime) {
    std::mt19937 rng(kSeed);
    gridiron::RandomSource random{rng};
    for (int i = 0; i < kSamples; ++i) {
        const gridiron::PlayOutcome outcome = gridiron::samplePuntOutcome(random);
        ASSERT_EQ(outcome.play_type, gridiron::PlayType::Punt);
        ASSERT_EQ(outcome.outcome_type, gridiron::OutcomeType::Punted);
        ASSERT_EQ(outcome.elapsed_seconds, gridiron::kPuntSeconds);
    }
}

TEST_F(OutcomeSamplerTest, PuntYardageMatchesConstants) {
    std::mt19937 rng(kSeed);
    gridiron::RandomSource random{rng};
    std::vector<int> yards;
    yards.reserve(kSamples);
    for (int i = 0; i < kSamples; ++i) {
        yards.push_back(gridiron::samplePuntOutcome(random).yards_gained);
    }
    EXPECT_NEAR(mean(yards), gridiron::kPuntNetMeanYards, 0.3);
    EXPECT_NEAR(stddev(yards), gridiron::kPuntNetSdYards, 0.3);
}

TEST_F(OutcomeSamplerTest, RunOutOfBoundsFrequencyMatchesRate) {
    std::mt19937 rng(kSeed);
    gridiron::RandomSource random{rng};
    int plays = 0;
    int outOfBounds = 0;
    for (int i = 0; i < kSamples; ++i) {
        const auto outcome = gridiron::sampleRunOutcome(offense, defense, random);
        if (outcome.outcome_type == gridiron::OutcomeType::Turnover) {
            ASSERT_FALSE(outcome.out_of_bounds);
            continue;
        }
        ++plays;
        if (outcome.out_of_bounds) {
            ++outOfBounds;
        }
    }
    EXPECT_NEAR(static_cast<double>(outOfBounds) / plays, gridiron::kOutOfBoundsRate, 0.02);
}

TEST_F(OutcomeSamplerTest, PassOutOfBoundsFrequencyMatchesRate) {
    std::mt19937 rng(kSeed);
    gridiron::RandomSource random{rng};
    int plays = 0;
    int outOfBounds = 0;
    for (int i = 0; i < kSamples; ++i) {
        const auto outcome = gridiron::samplePassOutcome(offense, defense, random);
        if (outcome.outcome_type == gridiron::OutcomeType::Turnover ||
            outcome.outcome_type == gridiron::OutcomeType::Incomplete) {
            ASSERT_FALSE(outcome.out_of_bounds);
            continue;
        }
        ++plays;
        if (outcome.out_of_bounds) {
            ++outOfBounds;
        }
    }
    EXPECT_NEAR(static_cast<double>(outOfBounds) / plays, gridiron::kOutOfBoundsRate, 0.02);
}

TEST_F(OutcomeSamplerTest, DefaultPolicyFieldGoalIsNotRandom) {
    std::mt19937 rng(42);
    const std::mt19937 original = rng;
    gridiron::RandomSource random{rng};

    const gridiron::DefaultPolicy policy;
    const gridiron::Matchup matchup{offense, defense};
    const gridiron::GameState state{.down = 4, .ball_position = 60};

    EXPECT_EQ(policy.choosePlay(state, matchup, random), gridiron::PlayType::FieldGoalAttempt);
    EXPECT_EQ(rng, original);
}

TEST_F(OutcomeSamplerTest, DefaultTimeoutStrategyUsesTeamScorePeriodAndPostPlayClock) {
    const gridiron::DefaultPolicy policy;
    gridiron::GameState state;
    state.home_score = 3;
    state.away_score = 7;
    const gridiron::PlayOutcome outcome{
        .play_type = gridiron::PlayType::Run, .outcome_type = gridiron::OutcomeType::Normal, .elapsed_seconds = 6};
    for (int quarter : {1, 2, 3, 4, 5}) {
        state.quarter = quarter;
        state.time_remaining_seconds = 126;
        EXPECT_EQ(policy.callTimeout(state, outcome, gridiron::Possession::Home),
                  quarter == 2 || quarter == 4 || quarter == 5);
        EXPECT_FALSE(policy.callTimeout(state, outcome, gridiron::Possession::Away));
        state.time_remaining_seconds = 127;
        EXPECT_FALSE(policy.callTimeout(state, outcome, gridiron::Possession::Home));
        state.time_remaining_seconds = 6;
        EXPECT_FALSE(policy.callTimeout(state, outcome, gridiron::Possession::Home));
    }
    state.quarter = 4;
    state.time_remaining_seconds = 100;
    state.home_score = 7;
    EXPECT_FALSE(policy.callTimeout(state, outcome, gridiron::Possession::Home));
    EXPECT_FALSE(policy.callTimeout(state, outcome, gridiron::Possession::Away));
}
