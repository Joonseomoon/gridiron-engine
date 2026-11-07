#include "GameRunner.h"

#include <stdexcept>

#include "ApplyOutcome.h"
#include "OutcomeSampler.h"
#include "PlayResult.h"
#include "RandomSource.h"

namespace gridiron {

GameState runGame(const TeamProfile& home, const TeamProfile& away, std::mt19937& rng) {
    return runGame(home, away, rng, PlayObserver{});
}

GameState runGame(const TeamProfile& home, const TeamProfile& away, std::mt19937& rng, const PlayObserver& onPlay) {
    GameState state;
    state.home = home;
    state.away = away;

    std::bernoulli_distribution coinToss(0.5);
    state.opening_possession = coinToss(rng) ? Possession::Home : Possession::Away;
    state.possession = state.opening_possession;

    // One random source per game, so no stored normal value carries over from another game.
    RandomSource random{rng};

    // Each sampled play consumes at least 10 seconds of a 900-second quarter.
    constexpr int maximumPlays = (900 / 10) * 4;
    int plays = 0;
    while (!state.game_over) {
        if (plays >= maximumPlays) {
            throw std::runtime_error("Game exceeded maximum play count");
        }
        const PlayOutcome outcome = samplePlayOutcome(state, random);
        if (onPlay) {
            const GameState before = state;

            const PlaySummary summary = applyOutcomeInPlace(state, outcome);

            const PlayResult result = {state, summary.event, summary.turnover, summary.period};

            onPlay(before, outcome, result);
        } else {
            applyOutcomeInPlace(state, outcome);
        }
        ++plays;
    }
    return state;
}

} // namespace gridiron
