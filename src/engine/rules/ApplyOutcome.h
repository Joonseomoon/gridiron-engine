#pragma once

#include "engine/core/GameState.h"
#include "engine/core/PlayOutcome.h"
#include "engine/core/PlayResult.h"

namespace gridiron {
PlaySummary applyOutcomeInPlace(GameState& state, const PlayOutcome& outcome);

PlayResult applyOutcome(const GameState& gameState, const PlayOutcome& playOutcome);

// True when the clock stops after this play, so no runoff: incomplete passes, scores, changes of possession,
// late out of bounds, and the two-minute warning.
bool clockStops(const GameState& before, const PlayOutcome& outcome);
} // namespace gridiron
