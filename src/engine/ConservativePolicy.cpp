#include "ConservativePolicy.h"
#include "DefaultPolicy.h"
#include "ModelConstants.h"

namespace gridiron {

PlayType ConservativePolicy::choosePlay(const GameState& state, const Matchup&, RandomSource& random) const {
    if (state.down == 4) {
        const int distanceToGoal = 100 - state.ball_position;
        return distanceToGoal <= kFieldGoalRangeYards ? PlayType::FieldGoalAttempt : PlayType::Punt;
    }
    return chooseRunOrPass(random);
}

bool ConservativePolicy::callTimeout(const GameState& before, const PlayOutcome& outcome, Possession team) const {
    return lateGame_.callTimeout(before, outcome, team);
}
} // namespace gridiron
