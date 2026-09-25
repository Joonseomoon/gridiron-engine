#include <gtest/gtest.h>

#include "../src/engine/OutcomeSampler.h"

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

TEST_F(OutcomeSamplerTest, SameSeedProducesSameRun){
    std::mt19937 rngOne(42);
    std::mt19937 rngTwo(42);
    gridiron::PlayOutcome outcomeOne = gridiron::sampleRunOutcome(offense, defense, rngOne);
    gridiron::PlayOutcome outcomeTwo = gridiron::sampleRunOutcome(offense, defense, rngTwo);
    EXPECT_EQ(outcomeOne.play_type, outcomeTwo.play_type);
    EXPECT_EQ(outcomeOne.outcome_type, outcomeTwo.outcome_type);
    EXPECT_EQ(outcomeOne.yards_gained, outcomeTwo.yards_gained);
    EXPECT_EQ(outcomeOne.elapsed_seconds, outcomeTwo.elapsed_seconds);
    
}

TEST_F(OutcomeSamplerTest, SameSeedProducesSamePass){
    std::mt19937 rngOne(42);
    std::mt19937 rngTwo(42);
    gridiron::PlayOutcome outcomeOne = gridiron::samplePassOutcome(offense, defense, rngOne);
    gridiron::PlayOutcome outcomeTwo = gridiron::samplePassOutcome(offense, defense, rngTwo);
    EXPECT_EQ(outcomeOne.play_type, outcomeTwo.play_type);
    EXPECT_EQ(outcomeOne.outcome_type, outcomeTwo.outcome_type);
    EXPECT_EQ(outcomeOne.yards_gained, outcomeTwo.yards_gained);
    EXPECT_EQ(outcomeOne.elapsed_seconds, outcomeTwo.elapsed_seconds);
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

TEST_F(OutcomeSamplerTest, RunPlayTimeMinimum){
    std::mt19937 rng(42);
    gridiron::PlayOutcome outcome = gridiron::sampleRunOutcome(offense, defense, rng);
    EXPECT_GE(outcome.elapsed_seconds, 10);
}

TEST_F(OutcomeSamplerTest, PassPlayTimeMinimum){
    std::mt19937 rng(42);
    gridiron::PlayOutcome outcome = gridiron::samplePassOutcome(offense, defense, rng);
    EXPECT_GE(outcome.elapsed_seconds, 10);
}

TEST_F(OutcomeSamplerTest, ChoosePlayTypeReturnsRunOrPass){
    std::mt19937 rng(42);
    gridiron::PlayType playType = gridiron::choosePlayType(rng);
    EXPECT_TRUE(playType == gridiron::PlayType::Run ||
                playType == gridiron::PlayType::Pass);
}