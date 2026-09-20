#pragma once

#include "GameState.h"
#include "PlayOutcome.h"

namespace gridiron {
    GameState applyOutcome(const GameState& gameState, const PlayOutcome& playOutcome);
}