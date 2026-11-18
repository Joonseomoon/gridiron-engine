#pragma once

#include "Policy.h"

namespace gridiron {

class DefaultPolicy : public Policy {
public:
    PlayType choosePlay(const GameState& state, const Matchup& teams, RandomSource& random) const override;

    bool callTimeout(const GameState& before, const PlayOutcome& outcome, Possession team) const override;
};
} // namespace gridiron
