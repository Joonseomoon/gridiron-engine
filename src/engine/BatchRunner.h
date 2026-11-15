#pragma once

#include "Matchup.h"
#include "SimulationResult.h"
#include <vector>

namespace gridiron {
std::vector<SimulationResult> runBatch(const Matchup& matchup, int games, std::uint32_t batchSeed, int threads = 1);
} // namespace gridiron
