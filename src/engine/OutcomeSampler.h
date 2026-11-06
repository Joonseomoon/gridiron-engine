#pragma once

#include "GameState.h"
#include "PlayOutcome.h"
#include "RandomSource.h"
#include "TeamProfile.h"

namespace gridiron {

PlayOutcome samplePlayOutcome(const GameState& gameState, RandomSource& random);

PlayType choosePlayType(const GameState& gameState, RandomSource& random);

PlayOutcome sampleRunOutcome(const TeamProfile& offense, const TeamProfile& defense, RandomSource& random);

PlayOutcome samplePassOutcome(const TeamProfile& offense, const TeamProfile& defense, RandomSource& random);

PlayOutcome sampleFieldGoalOutcome(const TeamProfile& kicker, int distanceToGoal, RandomSource& random);
} // namespace gridiron
