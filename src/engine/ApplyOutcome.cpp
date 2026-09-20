#include "ApplyOutcome.h"

namespace gridiron {
    GameState applyOutcome(const GameState& gameState, const PlayOutcome& playOutcome) {
        GameState newState= gameState;
        newState.ball_position += playOutcome.yards_gained;
        newState.distance -= playOutcome.yards_gained;
        newState.down++;
        newState.time_remaining_seconds -= playOutcome.elapsed_seconds;
        return newState;
    }
}