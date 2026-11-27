#pragma once

#include <string>
#include <vector>

#include "engine/core/GameState.h"

namespace gridiron {

// Every problem with a regulation scenario; empty if valid
// opening_possession is not checked: set it when starting in Q1 or Q2 (defaults to Home)
std::vector<std::string> validateScenario(const GameState& state);

} // namespace gridiron
