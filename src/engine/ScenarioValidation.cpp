#include "ScenarioValidation.h"

#include <string>
#include <vector>

namespace gridiron {

namespace {
// "down must be 1-4, got 5"
std::string rangeMessage(const std::string& field, int low, int high, int value) {
    return field + " must be " + std::to_string(low) + "-" + std::to_string(high) + ", got " + std::to_string(value);
}
} // namespace

std::vector<std::string> validateScenario(const GameState& state) {
    std::vector<std::string> problems;

    if (state.quarter < 1 || state.quarter > 4) {
        problems.push_back(rangeMessage("quarter", 1, 4, state.quarter));
    }
    if (state.time_remaining_seconds < 1 || state.time_remaining_seconds > 900) {
        problems.push_back(rangeMessage("time_remaining_seconds", 1, 900, state.time_remaining_seconds));
    }
    if (state.down < 1 || state.down > 4) {
        problems.push_back(rangeMessage("down", 1, 4, state.down));
    }

    const bool ballInRange = state.ball_position >= 1 && state.ball_position <= 99;
    if (!ballInRange) {
        problems.push_back(rangeMessage("ball_position", 1, 99, state.ball_position));
    }
    if (state.distance < 1) {
        problems.push_back("distance must be at least 1, got " + std::to_string(state.distance));
    } else if (ballInRange && state.distance > 100 - state.ball_position) {
        // Only checked against a valid ball position, so one mistake gives one message.
        problems.push_back("distance must be at most the " + std::to_string(100 - state.ball_position) +
                           " yards to the goal, got " + std::to_string(state.distance));
    }

    if (state.home_timeouts < 0 || state.home_timeouts > 3) {
        problems.push_back(rangeMessage("home_timeouts", 0, 3, state.home_timeouts));
    }
    if (state.away_timeouts < 0 || state.away_timeouts > 3) {
        problems.push_back(rangeMessage("away_timeouts", 0, 3, state.away_timeouts));
    }
    if (state.home_score < 0) {
        problems.push_back("home_score must not be negative, got " + std::to_string(state.home_score));
    }
    if (state.away_score < 0) {
        problems.push_back("away_score must not be negative, got " + std::to_string(state.away_score));
    }

    // Scenarios start in regulation, before the game has ended.
    if (state.game_over) {
        problems.push_back("game_over must be false");
    }
    if (state.overtime_possessions_completed != 0) {
        problems.push_back("overtime_possessions_completed must be 0, got " +
                           std::to_string(state.overtime_possessions_completed));
    }

    return problems;
}

} // namespace gridiron
