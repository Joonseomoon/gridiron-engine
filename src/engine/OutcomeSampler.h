#pragma once

#include "GameState.h"
#include "Matchup.h"
#include "PlayOutcome.h"
#include "RandomSource.h"
#include "TeamProfile.h"

namespace gridiron {

PlayOutcome samplePlayOutcome(const GameState& gameState, const Matchup& matchup, RandomSource& random);

PlayOutcome samplePlayOutcome(const GameState& gameState, const Matchup& matchup, PlayType playType,
                              RandomSource& random);

PlayOutcome sampleRunOutcome(const TeamProfile& offense, const TeamProfile& defense, RandomSource& random);

PlayOutcome samplePassOutcome(const TeamProfile& offense, const TeamProfile& defense, RandomSource& random);

PlayOutcome sampleFieldGoalOutcome(const TeamProfile& kicker, int distanceToGoal, RandomSource& random);

PlayOutcome samplePuntOutcome(RandomSource& random);
} // namespace gridiron
