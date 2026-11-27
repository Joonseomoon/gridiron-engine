#include "engine/policy/AggressivePolicy.h"
#include "engine/core/ModelConstants.h"
#include "engine/policy/DefaultPolicy.h"

namespace gridiron {

PlayType AggressivePolicy::choosePlay(const GameState& state, const Matchup& teams, RandomSource& random) const {
    const bool homeOnOffense = state.possession == Possession::Home;
    const int deficit = homeOnOffense ? state.away_score - state.home_score : state.home_score - state.away_score;
    const bool shortLateFourthDown = deficit >= 0 && state.quarter == 4 &&
                                     state.time_remaining_seconds <= kAggressiveWindowSeconds && state.down == 4 &&
                                     state.distance <= kAggressiveMaxDistance;

    if (shortLateFourthDown) {
        return chooseRunOrPass(random);
    }
    return lateGame_.choosePlay(state, teams, random);
}

bool AggressivePolicy::callTimeout(const GameState& before, const PlayOutcome& outcome, Possession team) const {
    return lateGame_.callTimeout(before, outcome, team);
}
} // namespace gridiron
