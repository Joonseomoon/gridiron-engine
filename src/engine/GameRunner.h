#pragma once

#include "GameState.h"
#include "TeamProfile.h"
#include "PlayOutcome.h"
#include "ApplyOutcome.h"

namespace gridiron{
    GameState runGame (const TeamProfile& home, const TeamProfile& away, std::mt19937& rng);
}