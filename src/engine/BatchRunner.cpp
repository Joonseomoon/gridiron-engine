#include "BatchRunner.h"
#include "GameRunner.h"

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <thread>

namespace gridiron {
std::vector<SimulationResult> runBatch(const TeamProfile& home, const TeamProfile& away, int games,
                                       const std::uint32_t batchSeed, int threads) {
    if (threads < 1) {
        throw std::invalid_argument("threads must be positive");
    }

    if (games <= 0) {
        return {};
    }

    std::vector<SimulationResult> results(static_cast<std::size_t>(games));

    const auto runRange = [&](int begin, int end) {
        for (int i = begin; i < end; ++i) {
            const std::uint32_t seed = batchSeed + static_cast<std::uint32_t>(i);
            std::mt19937 rng(seed);
            results[static_cast<std::size_t>(i)] = toResult(runGame(home, away, rng), seed);
        }
    };

    const int workers = std::min(threads, games);

    if (workers == 1) {
        runRange(0, games);
        return results;
    }

    std::vector<std::thread> pool;
    pool.reserve(static_cast<std::size_t>(workers));

    try {
        int begin = 0;

        for (int worker = 0; worker < workers; ++worker) {
            const int count = games / workers + (worker < games % workers ? 1 : 0);
            const int end = begin + count;

            pool.emplace_back(runRange, begin, end);
            begin = end;
        }
    } catch (...) {
        for (auto& worker : pool) {
            worker.join();
        }
        throw;
    }

    for (auto& worker : pool) {
        worker.join();
    }

    return results;
}
} // namespace gridiron
