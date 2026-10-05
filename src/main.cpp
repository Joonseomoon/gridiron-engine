#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "cli/CommandLine.h"
#include "engine/GameRunner.h"

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
    const gridiron::GameState result = gridiron::runGame(matchup.home, matchup.away, rng);

    std::cout << matchup.home.name << " " << result.home_score << " - " << matchup.away.name << " " << result.away_score
              << " (seed " << seed << ")\n";
    return 0;
}
