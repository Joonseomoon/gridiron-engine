#include "engine/rules/Overtime.h"
#include "engine/core/ModelConstants.h"

#include <random>

namespace gridiron {

void startOvertime(GameState& state, RandomSource& random) {
    state.quarter = 5;
    state.time_remaining_seconds = kOvertimeSeconds;
    state.home_timeouts = kOvertimeTimeouts;
    state.away_timeouts = kOvertimeTimeouts;
    state.overtime_possessions_completed = 0;
    state.game_over = false;

    std::bernoulli_distribution coinToss(0.5);
    state.possession = coinToss(random.rng) ? Possession::Home : Possession::Away;

    state.down = 1;
    state.distance = 10;
    state.ball_position = kKickoffSpot;
}

void finishOvertimePlay(const GameState& before, GameState& state, PlaySummary& summary) {
    if (before.quarter != 5) {
        return;
    }

    const bool homeScored = state.home_score > before.home_score;
    const bool awayScored = state.away_score > before.away_score;
    const bool scoreChanged = homeScored || awayScored;

    // A defensive score can leave possession unchanged after the modeled kickoff.
    const bool possessionEnded = state.possession != before.possession || scoreChanged;
    if (possessionEnded) {
        ++state.overtime_possessions_completed;
    }

    const bool defenseScored = before.possession == Possession::Home ? awayScored : homeScored;
    const bool bothCompleted = state.overtime_possessions_completed >= 2;
    const bool tied = state.home_score == state.away_score;

    if (defenseScored || (bothCompleted && !tied)) {
        state.game_over = true;
        summary.period = PeriodEvent::EndofGame;
    }
}
} // namespace gridiron
