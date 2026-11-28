#pragma once

#include <functional>
#include <optional>
#include <random>

#include "engine/core/GameState.h"
#include "engine/core/Matchup.h"
#include "engine/core/PlayOutcome.h"
#include "engine/core/PlayResult.h"
#include "engine/game/CandidateAction.h"
#include "engine/policy/Policy.h"

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

// Play from a supplied state to the end, including overtime if tied. No coin toss. firstAction, if set, replaces the
// policy's choice on the first play only
GameState resumeGame(GameState state, const Matchup& matchup, const Policies& policies,
                     std::optional<CandidateAction> firstAction, std::mt19937& rng, const PlayObserver& onPlay = {});

} // namespace gridiron
