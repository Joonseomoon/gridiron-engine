#pragma once

#include "GameState.h"
#include "PlayResult.h"
#include "RandomSource.h"

namespace gridiron {

// Called for a tied game after regulation. Preserves scores and regulation opening possession.
void startOvertime(GameState& state, RandomSource& random);

// Called after applying an overtime play, with its pre-play state. Preserves the actual final clock.
void finishOvertimePlay(const GameState& before, GameState& state, PlaySummary& summary);
} // namespace gridiron
