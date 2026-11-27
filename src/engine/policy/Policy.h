#pragma once

#include "engine/core/GameState.h"
#include "engine/core/Matchup.h"
#include "engine/core/PlayOutcome.h"
#include "engine/core/RandomSource.h"

namespace gridiron {

class Policy {
public:
    virtual ~Policy() = default;

    virtual PlayType choosePlay(const GameState& state, const Matchup& teams, RandomSource& random) const = 0;

    // Asked after sampling only if time remains, runoff would apply, and this team has a timeout.
    virtual bool callTimeout(const GameState& before, const PlayOutcome& outcome, Possession team) const = 0;
};

struct Policies {
    const Policy& home;
    const Policy& away;
};

} // namespace gridiron
