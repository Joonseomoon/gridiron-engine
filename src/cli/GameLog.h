#pragma once

#include "../engine/GameState.h"
#include "../engine/PlayOutcome.h"
#include "../engine/PlayResult.h"
#include "../engine/SimulationResult.h"
#include <ostream>
#include <string>

namespace gridiron {

void printPlay(std::ostream& out, const GameState& before, const PlayOutcome& outcome, const PlayResult& result);

void printFinalResult(std::ostream& out, const std::string& homeName, const std::string& awayName,
                      const SimulationResult& result);
} // namespace gridiron
