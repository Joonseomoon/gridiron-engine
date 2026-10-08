#include <cstdint>
#include <iostream>
#include <ostream>
#include <random>
#include <string>
#include <vector>

#include "cli/CommandLine.h"
#include "cli/GameLog.h"
#include "engine/GameRunner.h"
#include "engine/SimulationResult.h"

namespace {
constexpr const char* kUsage = "usage: simulate HOME AWAY [--seed N]\n";
}

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
    std::mt19937 rng(seed);
    const gridiron::GameState result = gridiron::runGame(
        matchup.home, matchup.away, rng,
        [](const gridiron::GameState& before, const gridiron::PlayOutcome& outcome,
           const gridiron::PlayResult& playResult) { gridiron::printPlay(std::cout, before, outcome, playResult); });

    gridiron::SimulationResult summary;
    summary.home_score = result.home_score;
    summary.away_score = result.away_score;
    summary.seed = seed;
    if (result.home_score > result.away_score) {
        summary.winner = gridiron::Winner::Home;
    } else if (result.home_score < result.away_score) {
        summary.winner = gridiron::Winner::Away;
    } else {
        summary.winner = gridiron::Winner::Tie;
    }

    gridiron::printFinalResult(std::cout, matchup.home.name, matchup.away.name, summary);
    return 0;
}
