#include "../src/cli/BatchCsv.h"
#include "../src/engine/BatchRunner.h"
#include "../src/engine/BatchSummary.h"

#include <gtest/gtest.h>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

TEST(BatchCsvTest, NormalReusltsTest) {
    gridiron::BatchMetadata metadata{{"BAL", 0.8, 0.8, 0.8, 0.7, 0.03, 0.94},
                                     {"SEA", 0.7, 0.8, 0.9, 0.8, 0.02, 0.95},
                                     3,
                                     1000u,
                                     "./build/simulate BAL SEA --games 3 --seed 1000 --out results.csv"};
    std::vector<gridiron::SimulationResult> result{
        {24, 17, gridiron::Winner::Home, 1000u},
        {10, 20, gridiron::Winner::Away, 1001u},
        {13, 13, gridiron::Winner::Tie, 1002u},
    };
    gridiron::BatchSummary summary = gridiron::aggregate(result);

    std::ostringstream out;
    gridiron::writeBatchCsv(out, metadata, result, summary);

    const std::string expected = "# model_version: m1-v1\n"
                                 "# home: BAL\n"
                                 "# home_offensive_pass_rating: 0.8\n"
                                 "# home_offensive_run_rating: 0.8\n"
                                 "# home_defensive_pass_rating: 0.8\n"
                                 "# home_defensive_run_rating: 0.7\n"
                                 "# home_turnover_rate: 0.03\n"
                                 "# home_field_goal_rating: 0.94\n"
                                 "# away: SEA\n"
                                 "# away_offensive_pass_rating: 0.7\n"
                                 "# away_offensive_run_rating: 0.8\n"
                                 "# away_defensive_pass_rating: 0.9\n"
                                 "# away_defensive_run_rating: 0.8\n"
                                 "# away_turnover_rate: 0.02\n"
                                 "# away_field_goal_rating: 0.95\n"
                                 "# games: 3\n"
                                 "# batch_seed: 1000\n"
                                 "# seed_policy: batchSeed + game (uint32 wrap)\n"
                                 "# command: ./build/simulate BAL SEA --games 3 --seed 1000 --out results.csv\n"
                                 "# home_wins: 1\n"
                                 "# away_wins: 1\n"
                                 "# ties: 1\n"
                                 "# mean_home_score: 15.67\n"
                                 "# mean_away_score: 16.67\n"
                                 "# mean_margin: -1.00\n"
                                 "# mean_total: 32.33\n"
                                 "game,seed,home_score,away_score,winner\n"
                                 "0,1000,24,17,home\n"
                                 "1,1001,10,20,away\n"
                                 "2,1002,13,13,tie\n";
    EXPECT_EQ(out.str(), expected);
}

TEST(BatchCsvTest, ReproduceableBatches) {
    gridiron::BatchMetadata metadata{{"BAL", 0.8, 0.8, 0.8, 0.7, 0.03, 0.94},
                                     {"SEA", 0.7, 0.8, 0.9, 0.8, 0.02, 0.95},
                                     3,
                                     1000u,
                                     "./build/simulate BAL SEA --games 3 --seed 1000 --out results.csv"};
    std::vector<gridiron::SimulationResult> firstResult =
        gridiron::runBatch(metadata.home, metadata.away, metadata.games, metadata.batchSeed);
    std::vector<gridiron::SimulationResult> secondResult =
        gridiron::runBatch(metadata.home, metadata.away, metadata.games, metadata.batchSeed);

    gridiron::BatchSummary firstSummary = gridiron::aggregate(firstResult);
    gridiron::BatchSummary secondSummary = gridiron::aggregate(secondResult);

    std::ostringstream firstOutput;
    std::ostringstream secondOutput;

    gridiron::writeBatchCsv(firstOutput, metadata, firstResult, firstSummary);
    gridiron::writeBatchCsv(secondOutput, metadata, secondResult, secondSummary);

    EXPECT_EQ(firstOutput.str(), secondOutput.str());
}

TEST(BatchCsvTest, RestoresStreamFormatting) {
    gridiron::BatchMetadata metadata{{"BAL", 0.8, 0.8, 0.8, 0.7, 0.03, 0.94},
                                     {"SEA", 0.7, 0.8, 0.9, 0.8, 0.02, 0.95},
                                     3,
                                     1000u,
                                     "./build/simulate BAL SEA --games 3 --seed 1000 --out results.csv"};
    std::vector<gridiron::SimulationResult> result =
        gridiron::runBatch(metadata.home, metadata.away, metadata.games, metadata.batchSeed);

    gridiron::BatchSummary summary = gridiron::aggregate(result);

    std::ostringstream output;
    output << std::scientific << std::setprecision(4);

    const auto originalFlags = output.flags();
    const auto originalPrecision = output.precision();

    gridiron::writeBatchCsv(output, metadata, result, summary);

    EXPECT_EQ(output.flags(), originalFlags);
    EXPECT_EQ(output.precision(), originalPrecision);
}
