#include "Timeout.h"
#include "ApplyOutcome.h"

namespace gridiron {

std::optional<Possession> applyTimeout(GameState& state, PlayOutcome& outcome, const Policies& policies) {

    if (state.game_over || state.time_remaining_seconds <= outcome.elapsed_seconds || outcome.runoff_seconds <= 0 ||
        clockStops(state, outcome)) {
        return std::nullopt;
    }

    const Possession offense = state.possession;
    const Possession defense = offense == Possession::Home ? Possession::Away : Possession::Home;

    for (Possession team : {offense, defense}) {
        int& remaining = team == Possession::Home ? state.home_timeouts : state.away_timeouts;
        const Policy& policy = team == Possession::Home ? policies.home : policies.away;

        if (remaining > 0 && policy.callTimeout(state, outcome, team)) {
            --remaining;
            outcome.runoff_seconds = 0;
            return team;
        }
    }
    return std::nullopt;
}
} // namespace gridiron
