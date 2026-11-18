#include "GameRunner.h"

#include <optional>
#include <stdexcept>

#include "ApplyOutcome.h"
#include "DefaultPolicy.h"
#include "ModelConstants.h"
#include "OutcomeSampler.h"
#include "Overtime.h"
#include "PlayResult.h"
#include "RandomSource.h"
#include "Timeout.h"

namespace gridiron {

GameState runGame(const Matchup& matchup, std::mt19937& rng) {
    return runGame(matchup, rng, PlayObserver{});
}

GameState runGame(const Matchup& matchup, std::mt19937& rng, const PlayObserver& onPlay) {
    const DefaultPolicy policy;
    const Policies policies{policy, policy};
    return runGame(matchup, rng, policies, onPlay);
}

GameState runGame(const Matchup& matchup, std::mt19937& rng, const Policies& policies, const PlayObserver& onPlay) {
    GameState state;

    std::bernoulli_distribution coinToss(0.5);
    state.opening_possession = coinToss(rng) ? Possession::Home : Possession::Away;
    state.possession = state.opening_possession;

    // One random source per game, so no stored normal value carries over from another game.
    RandomSource random{rng};

    // Each sampled play consumes at least kSnapSeconds seconds of a 900-second quarter.
    constexpr int maximumPlays = (900 / kSnapSeconds) * 4 + kOvertimeSeconds / kSnapSeconds;
    int plays = 0;
    while (!state.game_over) {
        if (plays >= maximumPlays) {
            throw std::runtime_error("Game exceeded maximum play count");
        }
        const Policy& policy = state.possession == Possession::Home ? policies.home : policies.away;
        const PlayType playType = policy.choosePlay(state, matchup, random);
        PlayOutcome outcome = samplePlayOutcome(state, matchup, playType, random);
        std::optional<GameState> before;
        if (onPlay || state.quarter == 5) {
            before = state;
        }

        const auto timeout = applyTimeout(state, outcome, policies);
        PlaySummary summary = applyOutcomeInPlace(state, outcome);

        if (before.has_value() && before->quarter == 5) {
            finishOvertimePlay(*before, state, summary);
        }

        if (state.game_over && state.quarter == 4 && state.home_score == state.away_score) {
            startOvertime(state, random);
            summary.period = PeriodEvent::OvertimeStart;
        }

        if (onPlay) {
            const PlayResult result{state, summary.event, summary.turnover, summary.period, timeout};
            onPlay(*before, outcome, result);
        }
        ++plays;
    }
    return state;
}

} // namespace gridiron
