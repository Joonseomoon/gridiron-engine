#include "cli/BatchLog.h"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <optional>
#include <ostream>
#include <random>
#include <string>
#include <vector>

#include "cli/BatchCsv.h"
#include "cli/CommandLine.h"
#include "cli/GameLog.h"
#include "engine/BatchMetadata.h"
#include "engine/BatchRunner.h"
#include "engine/BatchSummary.h"
#include "engine/GameRunner.h"
#include "engine/SimulationResult.h"

namespace {
constexpr const char* kUsage = "usage: simulate HOME AWAY [--seed N] [--games N] [--out FILE]\n";

std::string shellQuote(const std::string& argument) {
    std::string quoted = "'";
    for (char character : argument) {
        if (character == '\'') {
            quoted += "'\\''";
        } else {
            quoted += character;
        }
    }

    quoted += "'";
    return quoted;
}

void runSingleGame(const gridiron::Matchup& matchup, std::uint32_t seed) {
    std::mt19937 rng(seed);
    const gridiron::GameState result = gridiron::runGame(
        matchup.home, matchup.away, rng,
        [](const gridiron::GameState& before, const gridiron::PlayOutcome& outcome,
           const gridiron::PlayResult& playResult) { gridiron::printPlay(std::cout, before, outcome, playResult); });

    const auto summary = gridiron::toResult(result, seed);

    gridiron::printFinalResult(std::cout, matchup.home.name, matchup.away.name, summary);
}

int runBatchMode(const gridiron::Matchup& matchup, int games, std::uint32_t batchSeed,
                 const std::optional<std::string>& outputPath, const std::string& commandLine) {
    std::cout << matchup.home.name << " vs " << matchup.away.name << ", " << games << " games, batch seed " << batchSeed
              << "\n";
    const auto results = gridiron::runBatch(matchup.home, matchup.away, games, batchSeed);
    const auto summary = gridiron::aggregate(results);
    gridiron::printBatchSummary(std::cout, matchup.home.name, matchup.away.name, summary);

    if (outputPath) {
        gridiron::BatchMetadata metadata{matchup.home, matchup.away, games, batchSeed, commandLine};
        std::ofstream file(*outputPath);

        if (!file.is_open()) {
            std::cerr << "error: could not open file: " << *outputPath << '\n';
            return 1;
        }

        gridiron::writeBatchCsv(file, metadata, results, summary);

        file.close();

        if (!file) {
            std::cerr << "error: failed to write output file: " << *outputPath << '\n';
            return 1;
        }
    }

    return 0;
}
} // namespace

int main(int argc, char* argv[]) {
    const std::vector<std::string> args(argv + 1, argv + argc);

    std::string commandLine;
    for (int i = 0; i < argc; ++i) {
        if (i > 0) {
            commandLine += ' ';
        }
        commandLine += shellQuote(argv[i]);
    }

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
    if (!options.seed) {
        commandLine += " " + shellQuote("--seed") + " " + shellQuote(std::to_string(seed));
    }
    if (options.games) {
        return runBatchMode(matchup, *options.games, seed, options.out, commandLine);
    } else {
        runSingleGame(matchup, seed);
    }
    return 0;
}
