#pragma once

#include "GameState.h"
#include "PlayOutcome.h"
#include "TeamProfile.h"

#include <random>

namespace gridiron {

PlayOutcome samplePlayOutcome(const GameState& gameState, std::mt19937& rng);

PlayType choosePlayType(const GameState& gameState, std::mt19937& rng);

PlayOutcome sampleRunOutcome(const TeamProfile& offense, const TeamProfile& defense, std::mt19937& rng);

PlayOutcome samplePassOutcome(const TeamProfile& offense, const TeamProfile& defense, std::mt19937& rng);

PlayOutcome sampleFieldGoalOutcome(const TeamProfile& kicker, int distanceToGoal, std::mt19937& rng);
} // namespace gridiron
