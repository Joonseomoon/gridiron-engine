#include "engine/batch/BatchRunner.h"
#include "engine/batch/ParallelFor.h"
#include "engine/game/GameRunner.h"

#include <algorithm>
#include <cstddef>

namespace gridiron {
std::vector<SimulationResult> runBatch(const Matchup& matchup, int games, const std::uint32_t batchSeed, int threads) {
    std::vector<SimulationResult> results(static_cast<std::size_t>(std::max(games, 0)));

    const auto runRange = [&](int begin, int end) {
        for (int i = begin; i < end; ++i) {
            const std::uint32_t seed = batchSeed + static_cast<std::uint32_t>(i);
            std::mt19937 rng(seed);
            results[static_cast<std::size_t>(i)] = toResult(runGame(matchup, rng), seed);
        }
    };
    parallelFor(games, threads, runRange);
    return results;
}
} // namespace gridiron
