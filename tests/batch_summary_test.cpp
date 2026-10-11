#include "../src/engine/BatchSummary.h"

#include <gtest/gtest.h>
#include <vector>

TEST(BatchSummaryTest, RegularExample) {
    std::vector<gridiron::SimulationResult> results = {
        {24, 17, gridiron::Winner::Home, 7}, {10, 20, gridiron::Winner::Away, 8}, {13, 13, gridiron::Winner::Tie, 9}};

    const auto summary = gridiron::aggregate(results);

    EXPECT_EQ(summary.games, 3);
    EXPECT_EQ(summary.home_wins, 1);
    EXPECT_EQ(summary.away_wins, 1);
    EXPECT_EQ(summary.ties, 1);

    EXPECT_DOUBLE_EQ(summary.mean_home_score, 47.0 / 3.0);
    EXPECT_DOUBLE_EQ(summary.mean_away_score, 50.0 / 3.0);
    EXPECT_NEAR(summary.mean_margin, -1.0, 1e-12);
    EXPECT_DOUBLE_EQ(summary.mean_total, 97.0 / 3.0);

    EXPECT_EQ(summary.home_wins + summary.away_wins + summary.ties, summary.games);
}

TEST(BatchSummaryTest, EmptyVector) {
    std::vector<gridiron::SimulationResult> results = {};

    const auto summary = gridiron::aggregate(results);

    EXPECT_EQ(summary.games, 0);
    EXPECT_EQ(summary.home_wins, 0);
    EXPECT_EQ(summary.away_wins, 0);
    EXPECT_EQ(summary.ties, 0);

    EXPECT_DOUBLE_EQ(summary.mean_home_score, 0.0);
    EXPECT_DOUBLE_EQ(summary.mean_away_score, 0.0);
    EXPECT_DOUBLE_EQ(summary.mean_margin, 0.0);
    EXPECT_DOUBLE_EQ(summary.mean_total, 0.0);

    EXPECT_EQ(summary.home_wins + summary.away_wins + summary.ties, summary.games);
}

TEST(BatchSummaryTest, OneGame) {
    std::vector<gridiron::SimulationResult> results = {{24, 17, gridiron::Winner::Home, 7}};

    const auto summary = gridiron::aggregate(results);

    EXPECT_EQ(summary.games, 1);
    EXPECT_EQ(summary.home_wins, 1);
    EXPECT_EQ(summary.away_wins, 0);
    EXPECT_EQ(summary.ties, 0);

    EXPECT_DOUBLE_EQ(summary.mean_home_score, 24.0);
    EXPECT_DOUBLE_EQ(summary.mean_away_score, 17.0);
    EXPECT_DOUBLE_EQ(summary.mean_margin, 7.0);
    EXPECT_DOUBLE_EQ(summary.mean_total, 41.0);

    EXPECT_EQ(summary.home_wins + summary.away_wins + summary.ties, summary.games);
}

TEST(BatchSummaryTest, AllTies) {
    std::vector<gridiron::SimulationResult> results = {
        {24, 24, gridiron::Winner::Tie, 7}, {20, 20, gridiron::Winner::Tie, 8}, {13, 13, gridiron::Winner::Tie, 9}};

    const auto summary = gridiron::aggregate(results);

    EXPECT_EQ(summary.games, 3);
    EXPECT_EQ(summary.home_wins, 0);
    EXPECT_EQ(summary.away_wins, 0);
    EXPECT_EQ(summary.ties, 3);

    EXPECT_DOUBLE_EQ(summary.mean_home_score, 19.0);
    EXPECT_DOUBLE_EQ(summary.mean_away_score, 19.0);
    EXPECT_DOUBLE_EQ(summary.mean_margin, 0.0);
    EXPECT_DOUBLE_EQ(summary.mean_total, 38.0);

    EXPECT_EQ(summary.home_wins + summary.away_wins + summary.ties, summary.games);
}

TEST(BatchSummaryTest, AllHomeWins) {
    std::vector<gridiron::SimulationResult> results = {
        {30, 24, gridiron::Winner::Home, 7}, {20, 15, gridiron::Winner::Home, 8}, {10, 5, gridiron::Winner::Home, 9}};

    const auto summary = gridiron::aggregate(results);

    EXPECT_EQ(summary.games, 3);
    EXPECT_EQ(summary.home_wins, 3);
    EXPECT_EQ(summary.away_wins, 0);
    EXPECT_EQ(summary.ties, 0);

    EXPECT_DOUBLE_EQ(summary.mean_home_score, 20.0);
    EXPECT_DOUBLE_EQ(summary.mean_away_score, 44.0 / 3.0);
    EXPECT_NEAR(summary.mean_margin, 16.0 / 3.0, 1e-12);
    EXPECT_DOUBLE_EQ(summary.mean_total, 104.0 / 3.0);

    EXPECT_EQ(summary.home_wins + summary.away_wins + summary.ties, summary.games);
}

TEST(BatchSummaryTest, AllAwayWins) {
    std::vector<gridiron::SimulationResult> results = {
        {24, 30, gridiron::Winner::Away, 7}, {15, 20, gridiron::Winner::Away, 8}, {5, 10, gridiron::Winner::Away, 9}};

    const auto summary = gridiron::aggregate(results);

    EXPECT_EQ(summary.games, 3);
    EXPECT_EQ(summary.home_wins, 0);
    EXPECT_EQ(summary.away_wins, 3);
    EXPECT_EQ(summary.ties, 0);

    EXPECT_DOUBLE_EQ(summary.mean_home_score, 44.0 / 3.0);
    EXPECT_DOUBLE_EQ(summary.mean_away_score, 20.0);
    EXPECT_NEAR(summary.mean_margin, -16.0 / 3.0, 1e-12);
    EXPECT_DOUBLE_EQ(summary.mean_total, 104.0 / 3.0);

    EXPECT_EQ(summary.home_wins + summary.away_wins + summary.ties, summary.games);
}
