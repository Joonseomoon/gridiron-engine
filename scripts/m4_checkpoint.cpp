// Standalone checkpoint tool: see docs/decisions/M4.md for build/run commands.
#include "cli/BatchCsv.h"
#include "cli/BatchLog.h"
#include "cli/Teams.h"
#include "engine/BatchMetadata.h"
#include "engine/BatchRunner.h"
#include "engine/BatchSummary.h"
#include "engine/GameRunner.h"
#include <cstdint>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>

int main(int argc, char** argv) {
    try {
        if (argc != 7) {
            throw std::invalid_argument("usage: m4_checkpoint HOME AWAY GAMES SEED THREADS batch|csv|metrics");
        }
        const auto home = gridiron::findTeam(argv[1]);
        const auto away = gridiron::findTeam(argv[2]);
        const int games = std::stoi(argv[3]);
        const auto seed = static_cast<std::uint32_t>(std::stoul(argv[4]));
        const int threads = std::stoi(argv[5]);
        if (!home || !away || games < 1 || games > 1000000 || threads < 1) {
            throw std::invalid_argument("invalid team, games, or thread count");
        }
        const gridiron::Matchup matchup{*home, *away};
        const std::string mode = argv[6];
        if (mode == "batch" || mode == "csv") {
            const auto results = gridiron::runBatch(matchup, games, seed, threads);
            if (mode == "csv") {
                const gridiron::BatchMetadata metadata{*home, *away, games, seed, "m4_checkpoint", threads};
                gridiron::writeBatchCsv(std::cout, metadata, results, gridiron::aggregate(results));
                return 0;
            }
            std::cout << home->name << " vs " << away->name << ", " << games << " games, batch seed " << seed << '\n';
            gridiron::printBatchSummary(std::cout, home->name, away->name, gridiron::aggregate(results));
            if (games >= 100) {
                gridiron::printConvergence(std::cout, home->name, gridiron::convergence(results));
            }
            return 0;
        }
        if (mode != "metrics" || threads != 1) {
            throw std::invalid_argument("metrics mode requires one thread");
        }
        long long homeScore = 0, awayScore = 0, homePunts = 0, awayPunts = 0;
        long long attempts = 0, completions = 0, plays = 0, timeouts = 0;
        int ties = 0, overtime = 0;
        for (int i = 0; i < games; ++i) {
            std::mt19937 rng(seed + static_cast<std::uint32_t>(i));
            const auto final =
                gridiron::runGame(matchup, rng, [&](const auto& before, const auto& outcome, const auto& after) {
                    ++plays;
                    if (outcome.play_type == gridiron::PlayType::Punt) {
                        (before.possession == gridiron::Possession::Home ? homePunts : awayPunts)++;
                    }
                    if (outcome.play_type == gridiron::PlayType::Pass) {
                        ++attempts;
                        completions += outcome.outcome_type == gridiron::OutcomeType::Normal;
                    }
                    if (after.state.quarter == before.quarter) {
                        timeouts += after.state.home_timeouts < before.home_timeouts;
                        timeouts += after.state.away_timeouts < before.away_timeouts;
                    }
                });
            if (!final.game_over) {
                throw std::runtime_error("unfinished game");
            }
            homeScore += final.home_score;
            awayScore += final.away_score;
            ties += final.home_score == final.away_score;
            overtime += final.quarter == 5;
        }
        std::cout << "home,away,games,home_score,away_score,home_punts,away_punts,completion_rate,tie_rate,overtime_"
                     "rate,plays,timeouts\n"
                  << home->name << ',' << away->name << ',' << games << ',' << static_cast<double>(homeScore) / games
                  << ',' << static_cast<double>(awayScore) / games << ',' << static_cast<double>(homePunts) / games
                  << ',' << static_cast<double>(awayPunts) / games << ','
                  << (attempts ? static_cast<double>(completions) / static_cast<double>(attempts) : 0.0) << ','
                  << static_cast<double>(ties) / games << ',' << static_cast<double>(overtime) / games << ','
                  << static_cast<double>(plays) / games << ',' << static_cast<double>(timeouts) / games << '\n';
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
