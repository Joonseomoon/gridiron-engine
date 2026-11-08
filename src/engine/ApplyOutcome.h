#pragma once

#include "GameState.h"
#include "PlayOutcome.h"
#include "PlayResult.h"

namespace gridiron {
PlaySummary applyOutcomeInPlace(GameState& state, const PlayOutcome& outcome);

PlayResult applyOutcome(const GameState& gameState, const PlayOutcome& playOutcome);
} // namespace gridiron
