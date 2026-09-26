#include <gtest/gtest.h>

#include <cmath>
#include <vector>

#include "../src/engine/OutcomeSampler.h"

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
        gridiron::TeamProfile offense{
            "Offense",
            .8,
            .7,
            .8,
            .7,
            .01,
            .8
        };

        gridiron::TeamProfile defense{
            "Defense",
            .7,
            .9,
            .9,
            .5,
            .02,
            .8
        };
};

TEST_F(OutcomeSamplerTest, RunSamplerReturnsRun){
    std::mt19937 rng(42);
    gridiron::PlayOutcome outcome = gridiron::sampleRunOutcome(offense, defense, rng);
    EXPECT_EQ(outcome.play_type, gridiron::PlayType::Run);
}

TEST_F(OutcomeSamplerTest, PassSamplerReturnsPass){
    std::mt19937 rng(42);
    gridiron::PlayOutcome outcome = gridiron::samplePassOutcome(offense, defense, rng);
    EXPECT_EQ(outcome.play_type, gridiron::PlayType::Pass);
}

TEST_F(OutcomeSamplerTest, RunTurnoverRateOneAlwaysTurnover){
    std::mt19937 rng(42);
    offense.turnover_rate = 1.0;
    gridiron::PlayOutcome outcome = gridiron::sampleRunOutcome(offense, defense, rng);
    EXPECT_EQ(outcome.outcome_type, gridiron::OutcomeType::Turnover);
}

TEST_F(OutcomeSamplerTest, RunTurnoverRateZeroNeverTurnover){
    std::mt19937 rng(42);
    offense.turnover_rate = 0.0;
    gridiron::PlayOutcome outcome = gridiron::sampleRunOutcome(offense, defense, rng);
    EXPECT_EQ(outcome.outcome_type, gridiron::OutcomeType::Normal);
}

TEST_F(OutcomeSamplerTest, PassTurnoverRateOneAlwaysTurnover){
    std::mt19937 rng(42);
    offense.turnover_rate = 1.0;
    gridiron::PlayOutcome outcome = gridiron::samplePassOutcome(offense, defense, rng);
    EXPECT_EQ(outcome.outcome_type, gridiron::OutcomeType::Turnover);
}

TEST_F(OutcomeSamplerTest, PassTurnoverRateZeroNeverTurnover){
    std::mt19937 rng(42);
    offense.turnover_rate = 0.0;
    gridiron::PlayOutcome outcome = gridiron::samplePassOutcome(offense, defense, rng);
    EXPECT_EQ(outcome.outcome_type, gridiron::OutcomeType::Normal);
}

TEST_F(OutcomeSamplerTest, ChoosePlayTypeReturnsFieldGoalAttempt) {
    std::mt19937 rng(42);
    auto gameState = gridiron::GameState{.down = 4, .ball_position = 75};
    gridiron::PlayType playType = gridiron::choosePlayType(gameState, rng);
    EXPECT_EQ(playType, gridiron::PlayType::FieldGoalAttempt);
}

TEST_F(OutcomeSamplerTest, ChoosePlayTypeAttemptsFieldGoalAtExactly40) {
    std::mt19937 rng(42);
    auto gameState = gridiron::GameState{.down = 4, .ball_position = 60};
    gridiron::PlayType playType = gridiron::choosePlayType(gameState, rng);
    EXPECT_EQ(playType, gridiron::PlayType::FieldGoalAttempt);
}

TEST_F(OutcomeSamplerTest, ChoosePlayTypeReturnsRunOrPassOn4thDownOutOfFieldGoalRange) {
    std::mt19937 rng(42);
    auto gameState = gridiron::GameState{.down = 4, .ball_position = 59};
    gridiron::PlayType playType = gridiron::choosePlayType(gameState, rng);
    EXPECT_TRUE(playType == gridiron::PlayType::Run || playType == gridiron::PlayType::Pass);
}

TEST_F(OutcomeSamplerTest, ChoosePlayTypeReturnsRunOrPassInFieldGoalRangeBeforeFourthDown) {
    std::mt19937 rng(42);
    auto gameState = gridiron::GameState{.down = 3, .ball_position = 75};
    gridiron::PlayType playType = gridiron::choosePlayType(gameState, rng);
    EXPECT_TRUE(playType == gridiron::PlayType::Run || playType == gridiron::PlayType::Pass);
}

TEST_F(OutcomeSamplerTest, SamplePlayOutcomeDispatchesFieldGoalAttempt) {
    std::mt19937 rng(42);
    auto gameState = gridiron::GameState{
        .home = offense, .away = defense, .down = 4, .ball_position = 75};
    gridiron::PlayOutcome outcome = gridiron::samplePlayOutcome(gameState, rng);
    EXPECT_EQ(outcome.play_type, gridiron::PlayType::FieldGoalAttempt);
}

// --- Yardage formulas -------------------------------------------------------
// Single-sample tests cannot pin these down at all: they verify the base yards,
// the rating multipliers, which ratings feed which formula, the sign of the
// rating difference, the spread, and the rounding.

TEST_F(OutcomeSamplerTest, RunYardageMatchesExpectedMeanAndSpread) {
    // expected = 2 + 10 * (0.7 run offense - 0.5 run defense) = 4.0, sd 3.0
    std::mt19937 rng(kSeed);
    std::vector<int> yards;
    yards.reserve(kSamples);
    for (int i = 0; i < kSamples; ++i) {
        yards.push_back(gridiron::sampleRunOutcome(offense, defense, rng).yards_gained);
    }
    EXPECT_NEAR(mean(yards), 4.0, 0.2);
    EXPECT_NEAR(stddev(yards), 3.0, 0.2);
}

TEST_F(OutcomeSamplerTest, PassYardageMatchesExpectedMeanAndSpread) {
    // expected = 4 + 15 * (0.8 pass offense - 0.9 pass defense) = 2.5, sd 5.0
    std::mt19937 rng(kSeed);
    std::vector<int> yards;
    yards.reserve(kSamples);
    for (int i = 0; i < kSamples; ++i) {
        yards.push_back(gridiron::samplePassOutcome(offense, defense, rng).yards_gained);
    }
    EXPECT_NEAR(mean(yards), 2.5, 0.3);
    EXPECT_NEAR(stddev(yards), 5.0, 0.3);
}

// Catches a flipped rating difference, and catches the run formula reading pass
// ratings: both profiles below leave the pass ratings at zero, so a run formula
// using them would produce identical means.
TEST_F(OutcomeSamplerTest, BetterRunOffenseGainsMoreOnAverage) {
    gridiron::TeamProfile strong{.name = "Strong", .offensive_run_rating = 0.9};
    gridiron::TeamProfile weak{.name = "Weak", .offensive_run_rating = 0.3};

    std::mt19937 rng(kSeed);
    std::vector<int> strongYards;
    std::vector<int> weakYards;
    strongYards.reserve(kSamples);
    weakYards.reserve(kSamples);
    for (int i = 0; i < kSamples; ++i) {
        strongYards.push_back(gridiron::sampleRunOutcome(strong, defense, rng).yards_gained);
        weakYards.push_back(gridiron::sampleRunOutcome(weak, defense, rng).yards_gained);
    }
    EXPECT_GT(mean(strongYards), mean(weakYards));
}

TEST_F(OutcomeSamplerTest, BetterPassOffenseGainsMoreOnAverage) {
    gridiron::TeamProfile strong{.name = "Strong", .offensive_pass_rating = 0.9};
    gridiron::TeamProfile weak{.name = "Weak", .offensive_pass_rating = 0.3};

    std::mt19937 rng(kSeed);
    std::vector<int> strongYards;
    std::vector<int> weakYards;
    strongYards.reserve(kSamples);
    weakYards.reserve(kSamples);
    for (int i = 0; i < kSamples; ++i) {
        strongYards.push_back(gridiron::samplePassOutcome(strong, defense, rng).yards_gained);
        weakYards.push_back(gridiron::samplePassOutcome(weak, defense, rng).yards_gained);
    }
    EXPECT_GT(mean(strongYards), mean(weakYards));
}

// --- Turnovers --------------------------------------------------------------
// Rates of exactly 0 and 1 catch a wrong variable but not a wrong threshold.

TEST_F(OutcomeSamplerTest, RunTurnoverFrequencyMatchesRate) {
    gridiron::TeamProfile fumbler = offense;
    fumbler.turnover_rate = 0.3;

    std::mt19937 rng(kSeed);
    int turnovers = 0;
    for (int i = 0; i < kSamples; ++i) {
        if (gridiron::sampleRunOutcome(fumbler, defense, rng).outcome_type
            == gridiron::OutcomeType::Turnover) {
            ++turnovers;
        }
    }
    EXPECT_NEAR(turnovers / static_cast<double>(kSamples), 0.3, 0.02);
}

TEST_F(OutcomeSamplerTest, PassTurnoverFrequencyMatchesRate) {
    gridiron::TeamProfile fumbler = offense;
    fumbler.turnover_rate = 0.3;

    std::mt19937 rng(kSeed);
    int turnovers = 0;
    for (int i = 0; i < kSamples; ++i) {
        if (gridiron::samplePassOutcome(fumbler, defense, rng).outcome_type
            == gridiron::OutcomeType::Turnover) {
            ++turnovers;
        }
    }
    EXPECT_NEAR(turnovers / static_cast<double>(kSamples), 0.3, 0.02);
}

TEST_F(OutcomeSamplerTest, RunTurnoverStillReportsRunPlayType) {
    std::mt19937 rng(kSeed);
    offense.turnover_rate = 1.0;
    const gridiron::PlayOutcome outcome = gridiron::sampleRunOutcome(offense, defense, rng);
    EXPECT_EQ(outcome.outcome_type, gridiron::OutcomeType::Turnover);
    EXPECT_EQ(outcome.play_type, gridiron::PlayType::Run);
}

TEST_F(OutcomeSamplerTest, PassTurnoverStillReportsPassPlayType) {
    std::mt19937 rng(kSeed);
    offense.turnover_rate = 1.0;
    const gridiron::PlayOutcome outcome = gridiron::samplePassOutcome(offense, defense, rng);
    EXPECT_EQ(outcome.outcome_type, gridiron::OutcomeType::Turnover);
    EXPECT_EQ(outcome.play_type, gridiron::PlayType::Pass);
}

// --- Play duration ----------------------------------------------------------
// The floor holds ~98% of the time without its clamp, so one sample cannot tell
// the clamp apart from its absence. The mean checks catch a shifted distribution
// that the floor alone would hide.

TEST_F(OutcomeSamplerTest, NormalPlayTimeRespectsFloorAndMean) {
    // Fixture run yardage averages 4, so the over-20-yard branch never fires.
    std::mt19937 rng(kSeed);
    std::vector<int> times;
    times.reserve(kSamples);
    for (int i = 0; i < kSamples; ++i) {
        times.push_back(gridiron::sampleRunOutcome(offense, defense, rng).elapsed_seconds);
    }
    for (const int time : times) {
        ASSERT_GE(time, 10);
    }
    EXPECT_NEAR(mean(times), 20.0, 0.5);
}

TEST_F(OutcomeSamplerTest, BigPlayTimeUsesLongerDistribution) {
    // expected = 2 + 10 * (4.0 - 0.0) = 42 yards, so essentially every play
    // takes the over-20-yard branch, which the fixture profiles never reach.
    gridiron::TeamProfile explosive{.name = "Explosive", .offensive_run_rating = 4.0};
    gridiron::TeamProfile porous{.name = "Porous"};

    std::mt19937 rng(kSeed);
    std::vector<int> times;
    times.reserve(kSamples);
    for (int i = 0; i < kSamples; ++i) {
        const gridiron::PlayOutcome outcome = gridiron::sampleRunOutcome(explosive, porous, rng);
        ASSERT_GT(outcome.yards_gained, 20);
        times.push_back(outcome.elapsed_seconds);
    }
    for (const int time : times) {
        ASSERT_GE(time, 15);
    }
    EXPECT_NEAR(mean(times), 30.0, 0.5);
}

// --- Play type selection and dispatch ---------------------------------------

TEST_F(OutcomeSamplerTest, ChoosePlayTypeSplitsRunAndPassRoughlyEvenly) {
    std::mt19937 rng(kSeed);
    auto gameState = gridiron::GameState{.home = offense, .away = defense};
    int runs = 0;
    for (int i = 0; i < kSamples; ++i) {
        if (gridiron::choosePlayType(gameState, rng) == gridiron::PlayType::Run) {
            ++runs;
        }
    }
    EXPECT_NEAR(runs / static_cast<double>(kSamples), 0.5, 0.03);
}

TEST_F(OutcomeSamplerTest, SamplePlayOutcomeProducesBothRunAndPass) {
    std::mt19937 rng(kSeed);
    auto gameState = gridiron::GameState{.home = offense, .away = defense};
    int runs = 0;
    for (int i = 0; i < kSamples; ++i) {
        if (gridiron::samplePlayOutcome(gameState, rng).play_type == gridiron::PlayType::Run) {
            ++runs;
        }
    }
    EXPECT_NEAR(runs / static_cast<double>(kSamples), 0.5, 0.03);
}

// Two identically seeded generators consume the same first draw, so the decision
// is the same in both calls. A frequency check cannot catch a swapped dispatch:
// the split is symmetric and the reported play type follows whichever sampler
// ran, so swapping is invisible in aggregate.
TEST_F(OutcomeSamplerTest, SamplePlayOutcomeDispatchesToTheChosenPlayType) {
    auto gameState = gridiron::GameState{.home = offense, .away = defense};
    for (unsigned seed = 1; seed <= 50; ++seed) {
        std::mt19937 decisionRng(seed);
        std::mt19937 outcomeRng(seed);
        const gridiron::PlayType chosen = gridiron::choosePlayType(gameState, decisionRng);
        const gridiron::PlayOutcome outcome = gridiron::samplePlayOutcome(gameState, outcomeRng);
        EXPECT_EQ(outcome.play_type, chosen) << "seed " << seed;
    }
}

// --- Field goals ------------------------------------------------------------
// The clamp to [0.01, 0.99] means no rating guarantees a make or a miss, so the
// extremes have to be checked in aggregate rather than on a single draw.

TEST_F(OutcomeSamplerTest, FieldGoalMakeRateMatchesFormula) {
    // 0.95 rating, 23 to the goal -> 40 yard kick -> 0.95 - 0.01 * 20 = 0.75
    gridiron::TeamProfile kicker{.name = "Kicker", .field_goal_rating = 0.95};

    std::mt19937 rng(kSeed);
    int makes = 0;
    for (int i = 0; i < kSamples; ++i) {
        if (gridiron::sampleFieldGoalOutcome(kicker, 23, rng).outcome_type
            == gridiron::OutcomeType::FieldGoalMade) {
            ++makes;
        }
    }
    EXPECT_NEAR(makes / static_cast<double>(kSamples), 0.75, 0.02);
}

TEST_F(OutcomeSamplerTest, FieldGoalsConvertLessOftenFromLongerRange) {
    gridiron::TeamProfile kicker{.name = "Kicker", .field_goal_rating = 0.95};

    std::mt19937 rng(kSeed);
    int shortMakes = 0;
    int longMakes = 0;
    for (int i = 0; i < kSamples; ++i) {
        if (gridiron::sampleFieldGoalOutcome(kicker, 3, rng).outcome_type
            == gridiron::OutcomeType::FieldGoalMade) {
            ++shortMakes;
        }
        if (gridiron::sampleFieldGoalOutcome(kicker, 40, rng).outcome_type
            == gridiron::OutcomeType::FieldGoalMade) {
            ++longMakes;
        }
    }
    EXPECT_GT(shortMakes, longMakes);
}

TEST_F(OutcomeSamplerTest, BetterKickerConvertsMoreFromTheSameSpot) {
    gridiron::TeamProfile good{.name = "Good", .field_goal_rating = 0.95};
    gridiron::TeamProfile poor{.name = "Poor", .field_goal_rating = 0.60};

    std::mt19937 rng(kSeed);
    int goodMakes = 0;
    int poorMakes = 0;
    for (int i = 0; i < kSamples; ++i) {
        if (gridiron::sampleFieldGoalOutcome(good, 23, rng).outcome_type
            == gridiron::OutcomeType::FieldGoalMade) {
            ++goodMakes;
        }
        if (gridiron::sampleFieldGoalOutcome(poor, 23, rng).outcome_type
            == gridiron::OutcomeType::FieldGoalMade) {
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
    int makes = 0;
    int misses = 0;
    for (int i = 0; i < kSamples; ++i) {
        const gridiron::PlayOutcome outcome = gridiron::sampleFieldGoalOutcome(kicker, 23, rng);
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
    std::vector<int> times;
    times.reserve(kSamples);
    for (int i = 0; i < kSamples; ++i) {
        times.push_back(gridiron::sampleFieldGoalOutcome(kicker, 23, rng).elapsed_seconds);
    }
    for (const int time : times) {
        ASSERT_GE(time, 10);
    }
    EXPECT_NEAR(mean(times), 20.0, 0.5);
}
