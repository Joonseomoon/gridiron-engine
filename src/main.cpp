#include "cli/BatchLog.h"

#include <cstdint>
#include <iostream>
#include <ostream>
#include <random>
#include <string>
#include <vector>

#include "cli/CommandLine.h"
#include "cli/GameLog.h"
#include "engine/BatchRunner.h"
#include "engine/BatchSummary.h"
#include "engine/GameRunner.h"
#include "engine/SimulationResult.h"

namespace {
constexpr const char* kUsage = "usage: simulate HOME AWAY [--seed N] [--games N]\n";

void runSingleGame(const gridiron::Matchup& matchup, std::uint32_t seed) {
    std::mt19937 rng(seed);
    const gridiron::GameState result = gridiron::runGame(
        matchup.home, matchup.away, rng,
        [](const gridiron::GameState& before, const gridiron::PlayOutcome& outcome,
           const gridiron::PlayResult& playResult) { gridiron::printPlay(std::cout, before, outcome, playResult); });

    const auto summary = gridiron::toResult(result, seed);

    gridiron::printFinalResult(std::cout, matchup.home.name, matchup.away.name, summary);
}

void runBatchMode(const gridiron::Matchup& matchup, int games, std::uint32_t batchSeed) {
    std::cout << matchup.home.name << " vs " << matchup.away.name << ", " << games << " games, batch seed " << batchSeed
              << "\n";
    const auto results = gridiron::runBatch(matchup.home, matchup.away, games, batchSeed);
    const auto summary = gridiron::aggregate(results);
    gridiron::printBatchSummary(std::cout, matchup.home.name, matchup.away.name, summary);
}
} // namespace

int main(int argc, char* argv[]) {
    const std::vector<std::string> args(argv + 1, argv + argc);

    const auto parsed = gridiron::parseArgs(args);
    if (const auto* error = std::get_if<std::string>(&parsed)) {
        std::cerr << "error: " << *error << "\n" << kUsage;
        return 1;
    }
    const auto& options = std::get<gridiron::Options>(parsed);

    const auto resolved = gridiron::resolveTeams(options);
    if (const auto* error = std::get_if<std::string>(&resolved)) {
        std::cerr << "error: " << *error << "\n" << kUsage;
        return 1;
    }
    const auto& matchup = std::get<gridiron::Matchup>(resolved);

    const std::uint32_t seed = options.seed ? *options.seed : std::random_device{}();
    if (options.games) {
        runBatchMode(matchup, *options.games, seed);
    } else {
        runSingleGame(matchup, seed);
    }
    return 0;
}
