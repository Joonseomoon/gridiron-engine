#include "../src/engine/GameState.h"
#include "../src/engine/SimulationResult.h"
#include <gtest/gtest.h>

TEST(SimulationResultTest, HomeTeamWin) {
    const auto gameState = gridiron::GameState{.home_score = 17, .away_score = 16};
    const auto result = gridiron::toResult(gameState, 1000);

    EXPECT_EQ(result.home_score, 17);
    EXPECT_EQ(result.away_score, 16);
    EXPECT_EQ(result.seed, 1000);
    EXPECT_EQ(result.winner, gridiron::Winner::Home);
}

TEST(SimulationResultTest, AwayTeamWin) {
    const auto gameState = gridiron::GameState{.home_score = 16, .away_score = 17};
    const auto result = gridiron::toResult(gameState, 1000);

    EXPECT_EQ(result.home_score, 16);
    EXPECT_EQ(result.away_score, 17);
    EXPECT_EQ(result.seed, 1000);
    EXPECT_EQ(result.winner, gridiron::Winner::Away);
}

TEST(SimulationResultTest, TieGame) {
    const auto gameState = gridiron::GameState{.home_score = 16, .away_score = 16};
    const auto result = gridiron::toResult(gameState, 1000);

    EXPECT_EQ(result.home_score, 16);
    EXPECT_EQ(result.away_score, 16);
    EXPECT_EQ(result.seed, 1000);
    EXPECT_EQ(result.winner, gridiron::Winner::Tie);
}
