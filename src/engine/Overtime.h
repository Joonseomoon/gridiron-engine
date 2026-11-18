#pragma once

#include "GameState.h"
#include "PlayResult.h"
#include "RandomSource.h"

namespace gridiron {

void startOvertime(GameState& state, RandomSource& random);

void finishOvertimePlay(const GameState& before, GameState& state, PlaySummary& summary);
} // namespace gridiron
