#pragma once

#include <functional>
#include <random>

#include "GameState.h"
#include "Matchup.h"
#include "PlayOutcome.h"
#include "PlayResult.h"
#include "Policy.h"

namespace gridiron {

// Called once per play with the state before the snap, the sampled outcome,
// and the state after applying it. Outcome runoff reflects any called timeout.
using PlayObserver = std::function<void(const GameState& before, const PlayOutcome& outcome, const PlayResult& after)>;

// Simulate regulation and, if tied, one regular-season overtime period.
// Uses the caller's random engine; overtime can still end tied.
GameState runGame(const Matchup& matchup, std::mt19937& rng);

// Same as above, but reports every play to onPlay. Draws the same random
// numbers, so a given seed produces the same game with or without an observer.
GameState runGame(const Matchup& matchup, std::mt19937& rng, const PlayObserver& onPlay);

GameState runGame(const Matchup& matchup, std::mt19937& rng, const Policies& policies, const PlayObserver& onPlay);

GameState resumeGame(GameState state, const Matchup& matchup, const Policies& policies, std::mt19937& rng,
                     const PlayObserver& onPlay = {});

} // namespace gridiron
