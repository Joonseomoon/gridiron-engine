#pragma once

#include "LateGamePolicy.h"

namespace gridiron {

// Kicks on every 4th down: a field goal in range, a punt otherwise. Timeouts follow LateGamePolicy
class ConservativePolicy : public Policy {
public:
    PlayType choosePlay(const GameState& state, const Matchup& teams, RandomSource& random) const override;
    bool callTimeout(const GameState& before, const PlayOutcome& outcome, Possession team) const override;

private:
    LateGamePolicy lateGame_;
};
} // namespace gridiron
