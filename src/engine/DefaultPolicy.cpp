#include "DefaultPolicy.h"

#include <random>

namespace gridiron {

PlayType DefaultPolicy::choosePlay(const GameState& state, const Matchup&, RandomSource& random) const {
    const int distanceToGoal = 100 - state.ball_position;
    if (state.down == 4 && distanceToGoal <= 40) {
        return PlayType::FieldGoalAttempt;
    }

    std::uniform_real_distribution<double> passOrRunDistribution(0.0, 1.0);
    if (passOrRunDistribution(random.rng) > 0.5) {
        return PlayType::Run;
    }
    return PlayType::Pass;
}

bool DefaultPolicy::callTimeout(const GameState&, const PlayOutcome&, Possession) const {
    return false;
}

} // namespace gridiron
