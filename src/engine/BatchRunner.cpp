#include "BatchRunner.h"
#include "GameRunner.h"

#include <cstddef>

namespace gridiron {
std::vector<SimulationResult> runBatch(const TeamProfile& home, const TeamProfile& away, int games,
                                       const std::uint32_t batchSeed) {
    if (games <= 0) {
        return {};
    }

    std::vector<SimulationResult> results;
    results.reserve(static_cast<std::size_t>(games));

    for (int i = 0; i < games; ++i) {
        const std::uint32_t seed = batchSeed + static_cast<std::uint32_t>(i);
        std::mt19937 rng(seed);
        results.push_back(toResult(runGame(home, away, rng), seed));
    }

    return results;
}
} // namespace gridiron
