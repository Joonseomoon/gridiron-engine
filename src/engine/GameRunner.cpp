#include "GameRunner.h"

#include <stdexcept>

#include "OutcomeSampler.h"
#include "ApplyOutcome.h"

namespace gridiron {

GameState runGame(const TeamProfile& home, const TeamProfile& away, std::mt19937& rng) {
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
        state = applyOutcome(state, outcome);
        ++plays;
    }
    return state;
}

} // namespace gridiron
