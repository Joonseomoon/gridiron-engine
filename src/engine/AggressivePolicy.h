#pragma once

#include "LateGamePolicy.h"

namespace gridiron {

// Goes for it on short 4th downs late in Q4 when not leading. Otherwise, and for timeouts, follows LateGamePolicy
class AggressivePolicy : public Policy {
public:
    PlayType choosePlay(const GameState& state, const Matchup& teams, RandomSource& random) const override;
    bool callTimeout(const GameState& before, const PlayOutcome& outcome, Possession team) const override;

private:
    LateGamePolicy lateGame_;
};
} // namespace gridiron
