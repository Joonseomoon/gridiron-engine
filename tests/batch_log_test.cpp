#include "../src/cli/BatchLog.h"
#include "../src/engine/BatchSummary.h"

#include <gtest/gtest.h>
#include <iomanip>
#include <sstream>

TEST(BatchLogTest, RegularExample) {
    std::vector<gridiron::SimulationResult> results = {
        {24, 17, gridiron::Winner::Home, 7}, {10, 20, gridiron::Winner::Away, 8}, {13, 13, gridiron::Winner::Tie, 9}};

    const auto summary = gridiron::aggregate(results);

    std::ostringstream output;
    gridiron::printBatchSummary(output, "BAL", "SEA", summary);

    EXPECT_EQ(output.str(), "BAL wins: 1\tSEA wins: 1\tTies: 1\n"
                            "Mean score: BAL 15.67\tSEA 16.67\n"
                            "Mean margin (BAL - SEA): -1.00\tMean total: 32.33\n"
                            "Score sd: BAL 7.37\tSEA 3.51\tMargin sd: 8.54\tTotal sd: 7.77\n"
                            "95% CI: BAL win rate 0.333 ± 0.533\tMean margin -1.00 ± 9.67\n");
}

TEST(BatchLogTest, UsesProvidedNames) {
    std::vector<gridiron::SimulationResult> results = {{24, 17, gridiron::Winner::Home, 7}};

    const auto summary = gridiron::aggregate(results);

    std::ostringstream output;
    gridiron::printBatchSummary(output, "KC", "LAR", summary);

    EXPECT_EQ(output.str(), "KC wins: 1\tLAR wins: 0\tTies: 0\n"
                            "Mean score: KC 24.00\tLAR 17.00\n"
                            "Mean margin (KC - LAR): 7.00\tMean total: 41.00\n"
                            "Score sd: KC 0.00\tLAR 0.00\tMargin sd: 0.00\tTotal sd: 0.00\n"
                            "95% CI: KC win rate 1.000 ± 0.000\tMean margin 7.00 ± 0.00\n");
}

TEST(BatchLogTest, AllHomeWins) {
    std::vector<gridiron::SimulationResult> results = {
        {30, 24, gridiron::Winner::Home, 7}, {20, 15, gridiron::Winner::Home, 8}, {10, 5, gridiron::Winner::Home, 9}};

    const auto summary = gridiron::aggregate(results);

    std::ostringstream output;
    gridiron::printBatchSummary(output, "BAL", "SEA", summary);

    EXPECT_EQ(output.str(), "BAL wins: 3\tSEA wins: 0\tTies: 0\n"
                            "Mean score: BAL 20.00\tSEA 14.67\n"
                            "Mean margin (BAL - SEA): 5.33\tMean total: 34.67\n"
                            "Score sd: BAL 10.00\tSEA 9.50\tMargin sd: 0.58\tTotal sd: 19.50\n"
                            "95% CI: BAL win rate 1.000 ± 0.000\tMean margin 5.33 ± 0.65\n");
}

TEST(BatchLogTest, AllAwayWins) {
    std::vector<gridiron::SimulationResult> results = {
        {24, 30, gridiron::Winner::Away, 7}, {15, 20, gridiron::Winner::Away, 8}, {5, 10, gridiron::Winner::Away, 9}};

    const auto summary = gridiron::aggregate(results);

    std::ostringstream output;
    gridiron::printBatchSummary(output, "BAL", "SEA", summary);

    EXPECT_EQ(output.str(), "BAL wins: 0\tSEA wins: 3\tTies: 0\n"
                            "Mean score: BAL 14.67\tSEA 20.00\n"
                            "Mean margin (BAL - SEA): -5.33\tMean total: 34.67\n"
                            "Score sd: BAL 9.50\tSEA 10.00\tMargin sd: 0.58\tTotal sd: 19.50\n"
                            "95% CI: BAL win rate 0.000 ± 0.000\tMean margin -5.33 ± 0.65\n");
}

TEST(BatchLogTest, AllTies) {
    std::vector<gridiron::SimulationResult> results = {
        {24, 24, gridiron::Winner::Tie, 7}, {20, 20, gridiron::Winner::Tie, 8}, {13, 13, gridiron::Winner::Tie, 9}};

    const auto summary = gridiron::aggregate(results);

    std::ostringstream output;
    gridiron::printBatchSummary(output, "BAL", "SEA", summary);

    EXPECT_EQ(output.str(), "BAL wins: 0\tSEA wins: 0\tTies: 3\n"
                            "Mean score: BAL 19.00\tSEA 19.00\n"
                            "Mean margin (BAL - SEA): 0.00\tMean total: 38.00\n"
                            "Score sd: BAL 5.57\tSEA 5.57\tMargin sd: 0.00\tTotal sd: 11.14\n"
                            "95% CI: BAL win rate 0.000 ± 0.000\tMean margin 0.00 ± 0.00\n");
}

TEST(BatchLogTest, RestoreStreamFormatting) {
    std::vector<gridiron::SimulationResult> results = {{24, 17, gridiron::Winner::Home, 7}};

    const auto summary = gridiron::aggregate(results);

    std::ostringstream output;
    output << std::scientific << std::setprecision(4);
    const auto originalFlags = output.flags();
    const auto originalPrecision = output.precision();

    gridiron::printBatchSummary(output, "BAL", "SEA", summary);

    EXPECT_EQ(output.str(), "BAL wins: 1\tSEA wins: 0\tTies: 0\n"
                            "Mean score: BAL 24.00\tSEA 17.00\n"
                            "Mean margin (BAL - SEA): 7.00\tMean total: 41.00\n"
                            "Score sd: BAL 0.00\tSEA 0.00\tMargin sd: 0.00\tTotal sd: 0.00\n"
                            "95% CI: BAL win rate 1.000 ± 0.000\tMean margin 7.00 ± 0.00\n");
    EXPECT_EQ(output.flags(), originalFlags);
    EXPECT_EQ(output.precision(), originalPrecision);
}
