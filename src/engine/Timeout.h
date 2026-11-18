#pragma once

#include <optional>

#include "GameState.h"
#include "PlayOutcome.h"
#include "Policy.h"

namespace gridiron {

std::optional<Possession> applyTimeout(GameState& state, PlayOutcome& outcome, const Policies& policies);
}
