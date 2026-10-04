#pragma once

#include <functional>
#include <random>

#include "GameState.h"
#include "PlayOutcome.h"
#include "TeamProfile.h"

namespace gridiron {

// Called once per play with the state before the snap, the sampled outcome,
// and the state after applying it.
using PlayObserver = std::function<void(const GameState& before, const PlayOutcome& outcome, const GameState& after)>;

// Simulate four quarters using the caller's random engine; ties are allowed.
GameState runGame(const TeamProfile& home, const TeamProfile& away, std::mt19937& rng);

// Same as above, but reports every play to onPlay. Draws the same random
// numbers, so a given seed produces the same game with or without an observer.
GameState runGame(const TeamProfile& home, const TeamProfile& away, std::mt19937& rng, const PlayObserver& onPlay);

} // namespace gridiron
