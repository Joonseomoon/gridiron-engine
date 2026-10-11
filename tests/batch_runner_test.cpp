#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>

#include "../src/engine/BatchRunner.h"
#include "../src/engine/GameRunner.h"

namespace {
const gridiron::TeamProfile kHome{"BAL", 0.8, 0.7, 0.8, 0.7, 0.05, 0.95};
const gridiron::TeamProfile kAway{"SEA", 0.7, 0.9, 0.9, 0.5, 0.05, 0.95};
} // namespace

TEST(BatchRunnerTest, EachGameMatchesStandaloneGameWithItsSeed) {
    const std::uint32_t batchSeed = 1000;
    const auto results = gridiron::runBatch(kHome, kAway, 10, batchSeed);
    ASSERT_EQ(results.size(), 10u);

    for (std::size_t i = 0; i < results.size(); ++i) {
        SCOPED_TRACE(i);
        const std::uint32_t seed = batchSeed + static_cast<std::uint32_t>(i);
        std::mt19937 rng(seed);
        const auto alone = gridiron::runGame(kHome, kAway, rng);

        EXPECT_EQ(results[i].home_score, alone.home_score);
        EXPECT_EQ(results[i].away_score, alone.away_score);
        EXPECT_EQ(results[i].seed, seed);
    }
}

TEST(BatchRunnerTest, RunBatchTwiceWithSameArgumentsIsSame) {
    const std::uint32_t batchSeed = 1000;
    const auto results1 = gridiron::runBatch(kHome, kAway, 10, batchSeed);
    const auto results2 = gridiron::runBatch(kHome, kAway, 10, batchSeed);
    ASSERT_EQ(results1.size(), 10u);
    ASSERT_EQ(results2.size(), 10u);

    for (std::size_t i = 0; i < results1.size(); ++i) {
        SCOPED_TRACE(i);

        EXPECT_EQ(results1[i].home_score, results2[i].home_score);
        EXPECT_EQ(results1[i].away_score, results2[i].away_score);
        EXPECT_EQ(results1[i].seed, results2[i].seed);
    }
}

TEST(BatchRunnerTest, RunBatchWithLargeSeed) {
    const std::uint32_t batchSeed = std::numeric_limits<std::uint32_t>::max();
    const auto results = gridiron::runBatch(kHome, kAway, 3, batchSeed);
    ASSERT_EQ(results.size(), 3u);

    EXPECT_EQ(results[0].seed, batchSeed);
    EXPECT_EQ(results[1].seed, 0);
    EXPECT_EQ(results[2].seed, 1);
}

TEST(BatchRunnerTest, GameCountEdgeCases) {
    const std::uint32_t batchSeed = 1000;
    const auto results1 = gridiron::runBatch(kHome, kAway, 0, batchSeed);
    EXPECT_TRUE(results1.empty());
    const auto results2 = gridiron::runBatch(kHome, kAway, -3, batchSeed);
    EXPECT_TRUE(results2.empty());
    const auto results3 = gridiron::runBatch(kHome, kAway, 5, batchSeed);
    EXPECT_EQ(results3.size(), 5u);
}
