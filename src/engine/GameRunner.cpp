#include "GameRunner.h"

#include <stdexcept>

#include "ApplyOutcome.h"
#include "OutcomeSampler.h"
#include "PlayResult.h"

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

    // Each sampled play consumes at least 10 seconds of a 900-second quarter.
    constexpr int maximumPlays = (900 / 10) * 4;
    int plays = 0;
    while (!state.game_over) {
        if (plays >= maximumPlays) {
            throw std::runtime_error("Game exceeded maximum play count");
        }
        const PlayOutcome outcome = samplePlayOutcome(state, rng);
        PlayResult result = applyOutcome(state, outcome);
        if (onPlay) {
            onPlay(state, outcome, result);
        }
        state = result.state;
        ++plays;
    }
    return state;
}

} // namespace gridiron
