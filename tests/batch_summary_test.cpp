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