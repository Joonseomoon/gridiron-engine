#include "LateGamePolicy.h"
#include "DefaultPolicy.h"
#include "ModelConstants.h"

namespace gridiron {

PlayType LateGamePolicy::choosePlay(const GameState& state, const Matchup& teams, RandomSource& random) const {
    const bool homeOnOffense = state.possession == Possession::Home;
    const int distanceToGoal = 100 - state.ball_position;
    const int deficit = homeOnOffense ? state.away_score - state.home_score : state.home_score - state.away_score;
    bool lateFourthDown =
        deficit > 0 && state.quarter == 4 && state.time_remaining_seconds <= kTimeoutWindowSeconds && state.down == 4;

    if (lateFourthDown) {
        if (distanceToGoal <= kFieldGoalRangeYards && deficit <= 3) {
            return PlayType::FieldGoalAttempt;
        }
        return chooseRunOrPass(random);
    }
    return DefaultPolicy{}.choosePlay(state, teams, random);
}

bool LateGamePolicy::callTimeout(const GameState& before, const PlayOutcome& outcome, Possession team) const {
    if (before.quarter != 4) {
        return DefaultPolicy{}.callTimeout(before, outcome, team);
    }

    const int secondsLeft = before.time_remaining_seconds - outcome.elapsed_seconds;
    const int timeoutWindow = (team == before.possession) ? kOffenseTimeoutWindowSeconds : kTimeoutWindowSeconds;

    const bool isTrailing =
        (team == Possession::Home) ? before.away_score > before.home_score : before.home_score > before.away_score;

    return isTrailing && secondsLeft > 0 && secondsLeft <= timeoutWindow;
}
} // namespace gridiron
