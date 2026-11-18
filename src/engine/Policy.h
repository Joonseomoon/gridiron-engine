#pragma once

#include "GameState.h"
#include "Matchup.h"
#include "PlayOutcome.h"
#include "RandomSource.h"

namespace gridiron {

class Policy {
public:
    virtual ~Policy() = default;

    virtual PlayType choosePlay(const GameState& state, const Matchup& teams, RandomSource& random) const = 0;

    virtual bool callTimeout(const GameState& before, const PlayOutcome& outcome, Possession team) const = 0;
};

struct Policies {
    const Policy& home;
    const Policy& away;
};

} // namespace gridiron
