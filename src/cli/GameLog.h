#pragma once

#include "../engine/GameState.h"
#include "../engine/PlayOutcome.h"
#include "../engine/PlayResult.h"
#include <ostream>

namespace gridiron {

void printPlay(std::ostream& out, const GameState& before, const PlayOutcome& outcome, const PlayResult& result);

}