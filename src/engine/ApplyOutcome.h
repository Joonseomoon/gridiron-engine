#pragma once

#include "GameState.h"
#include "PlayOutcome.h"
#include "PlayResult.h"

namespace gridiron {
PlayResult applyOutcome(const GameState& gameState, const PlayOutcome& playOutcome);
}
