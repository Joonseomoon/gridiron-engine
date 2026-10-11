#include "SimulationResult.h"

namespace gridiron {
SimulationResult toResult(const GameState& gameState, std::uint32_t seed) {
    SimulationResult result;
    result.home_score = gameState.home_score;
    result.away_score = gameState.away_score;
    result.seed = seed;
    if (gameState.home_score > gameState.away_score) {
        result.winner = Winner::Home;
    } else if (gameState.home_score < gameState.away_score) {
        result.winner = Winner::Away;
    } else {
        result.winner = Winner::Tie;
    }
    return result;
}

} // namespace gridiron
