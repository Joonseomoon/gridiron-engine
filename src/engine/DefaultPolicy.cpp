#include "DefaultPolicy.h"
#include "ModelConstants.h"

#include <random>

namespace gridiron {

PlayType DefaultPolicy::choosePlay(const GameState& state, const Matchup&, RandomSource& random) const {
    const int distanceToGoal = 100 - state.ball_position;
    if (state.down == 4) {
        if (distanceToGoal <= kFieldGoalRangeYards) {
            return PlayType::FieldGoalAttempt;
        }
        if (state.distance > kFourthDownGoForItMaxDistance) {
            return PlayType::Punt;
        }
    }

    return chooseRunOrPass(random);
}

bool DefaultPolicy::callTimeout(const GameState& before, const PlayOutcome& outcome, Possession team) const {
    const int afterPlay = before.time_remaining_seconds - outcome.elapsed_seconds;
    const bool latePeriod = before.quarter == 2 || before.quarter == 4 || before.quarter == 5;
    const int ownScore = team == Possession::Home ? before.home_score : before.away_score;
    const int opponentScore = team == Possession::Home ? before.away_score : before.home_score;
    // A simple initial strategy: preserve time when trailing late in a half or overtime.
    // The engine checks remaining timeouts and whether the clock already stops.
    return latePeriod && afterPlay > 0 && afterPlay <= kTimeoutWindowSeconds && ownScore < opponentScore;
}

PlayType chooseRunOrPass(RandomSource& random) {
    std::uniform_real_distribution<double> passOrRunDistribution(0.0, 1.0);
    if (passOrRunDistribution(random.rng) > 0.5) {
        return PlayType::Run;
    }
    return PlayType::Pass;
}
} // namespace gridiron
