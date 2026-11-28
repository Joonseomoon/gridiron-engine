#pragma once

#include "engine/batch/SimulationResult.h"
#include "engine/core/GameState.h"
#include "engine/core/Matchup.h"
#include "engine/core/PlayOutcome.h"
#include "engine/core/PlayResult.h"
#include <ostream>
#include <string>

namespace gridiron {

void printPlay(std::ostream& out, const GameState& before, const Matchup& matchup, const PlayOutcome& outcome,
               const PlayResult& result);

void printFinalResult(std::ostream& out, const std::string& homeName, const std::string& awayName,
                      const SimulationResult& result);
} // namespace gridiron
