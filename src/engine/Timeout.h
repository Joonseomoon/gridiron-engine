#pragma once

#include <optional>

#include "GameState.h"
#include "PlayOutcome.h"
#include "Policy.h"

namespace gridiron {

// Before applying the play: remove runoff and charge at most one eligible team.
// Returns the caller for logging; does not advance the game clock.
std::optional<Possession> applyTimeout(GameState& state, PlayOutcome& outcome, const Policies& policies);
} // namespace gridiron
