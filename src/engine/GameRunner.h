#pragma once

#include <random>

#include "GameState.h"
#include "TeamProfile.h"

namespace gridiron {

// Simulate four quarters using the caller's random engine; ties are allowed.
GameState runGame(const TeamProfile& home, const TeamProfile& away, std::mt19937& rng);

} // namespace gridiron
