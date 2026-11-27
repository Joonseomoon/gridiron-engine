#pragma once

#include <random>

namespace gridiron {
// A game's random source. Reusing one normal_distribution keeps the second value of each generated pair for the next
// draw. Create one per game so no stored value carries into another game
struct RandomSource {
    std::mt19937& rng;
    std::normal_distribution<double> normal{};
};
} // namespace gridiron
