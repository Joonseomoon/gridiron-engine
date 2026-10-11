#pragma once

#include "SimulationResult.h"
#include "TeamProfile.h"
#include <vector>

namespace gridiron {
std::vector<SimulationResult> runBatch(const TeamProfile& home, const TeamProfile& away, int games,
                                       std::uint32_t batchSeed);
} // namespace gridiron
