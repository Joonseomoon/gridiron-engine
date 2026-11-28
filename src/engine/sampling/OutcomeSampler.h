#pragma once

#include "engine/core/GameState.h"
#include "engine/core/Matchup.h"
#include "engine/core/PlayOutcome.h"
#include "engine/core/RandomSource.h"
#include "engine/core/TeamProfile.h"

namespace gridiron {
PlayOutcome samplePlayOutcome(const GameState& gameState, const Matchup& matchup, PlayType playType,
                              RandomSource& random);

PlayOutcome sampleRunOutcome(const TeamProfile& offense, const TeamProfile& defense, RandomSource& random);

PlayOutcome samplePassOutcome(const TeamProfile& offense, const TeamProfile& defense, RandomSource& random);

PlayOutcome sampleFieldGoalOutcome(const TeamProfile& kicker, int distanceToGoal, RandomSource& random);

PlayOutcome samplePuntOutcome(RandomSource& random);
} // namespace gridiron
