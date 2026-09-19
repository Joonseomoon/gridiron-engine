#pragma once

#include <cstdint>

namespace gridiron {

enum class Winner { Home, Away, Tie };

struct SimulationResult {
    int home_score = 0;
    int away_score = 0;
    Winner winner = Winner::Tie;
    uint64_t seed = 0;
};

} // namespace gridiron
