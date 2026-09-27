#include <random>

#include "GameRunner.h"

namespace gridiron{
    GameState runGame (const TeamProfile& home, const TeamProfile& away, std::mt19937& rng){
    gridiron::GameState state;
    state.home = home;
    state.away = away;
    std::binomial_distribution<int> coinToss(1, 0.5);
    if (coinToss(rng)){
        state.opening_possession = Possession::Home;
    }
    else{
        state.opening_possession = Possession::Away;
    }
    state.possession = state.opening_possession;

    while(state.game_over == false){
        gridiron::PlayOutcome outcome = gridiron::samplePlayOutcome(state, rng);
        gridiron::GameState newState = gridiron::applyOutcome(state, outcome);
        state = newState;
    }
    return state;
    }
}