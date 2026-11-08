#include "ApplyOutcome.h"
#include <algorithm>

namespace gridiron {
Possession opponentOf(const Possession possession) {
    if (possession == Possession::Away) {
        return Possession::Home;
    } else {
        return Possession::Away;
    }
}

void awardScoreAndKickoff(GameState& state, const Possession scoringTeam, const int points,
                          const Possession receivingTeam) {
    if (scoringTeam == Possession::Home) {
        state.home_score += points;
    } else {
        state.away_score += points;
    }

    state.possession = receivingTeam;
    state.ball_position = 25;
    state.down = 1;
    state.distance = 10;
}

void turnoverAtSpot(GameState& state) {
    state.possession = opponentOf(state.possession);
    state.ball_position = 100 - state.ball_position;
    state.down = 1;
    state.distance = std::min(10, 100 - state.ball_position);
}

void touchback(GameState& state) {
    state.possession = opponentOf(state.possession);
    state.ball_position = 25;
    state.down = 1;
    state.distance = 10;
}

PlaySummary applyOutcomeInPlace(GameState& state, const PlayOutcome& outcome) {
    PlaySummary result;

    switch (outcome.outcome_type) {
    case OutcomeType::Normal:
        state.ball_position += outcome.yards_gained;
        if (state.ball_position >= 100) {
            // A gain that reaches or crosses the goal line scores,
            // regardless of down/distance.
            result.event = PlayEvent::TouchDown;
            result.turnover = TurnoverKind::None;
            awardScoreAndKickoff(state, state.possession, 7, opponentOf(state.possession));
        } else if (state.ball_position <= 0) {
            // Tackled in the offense's own end zone: safety.
            result.event = PlayEvent::Safety;
            result.turnover = TurnoverKind::None;
            awardScoreAndKickoff(state, opponentOf(state.possession), 2, opponentOf(state.possession));
        } else if (outcome.yards_gained >= state.distance) {
            // First down conversion
            result.event = PlayEvent::FirstDown;
            result.turnover = TurnoverKind::None;
            state.distance = std::min(10, 100 - state.ball_position);
            state.down = 1;
        } else if (state.down == 4 && outcome.yards_gained < state.distance) {
            result.event = PlayEvent::Normal; // Check in on this why is it normal would it make sense to be a turnover?
            result.turnover = TurnoverKind::TurnoverOnDowns;
            turnoverAtSpot(state);
        } else {
            result.event = PlayEvent::Normal;
            result.turnover = TurnoverKind::None;
            state.distance -= outcome.yards_gained;
            state.down++;
        }
        break;
    case OutcomeType::Turnover:
        if (outcome.play_type == PlayType::Pass) {
            result.turnover = TurnoverKind::Interception;
        } else if (outcome.play_type == PlayType::Run) {
            result.turnover = TurnoverKind::Fumble;
        } else {
            result.turnover = TurnoverKind::FieldGoalMissed;
        }

        if (outcome.play_type != PlayType::FieldGoalAttempt) {
            state.ball_position += outcome.yards_gained;
        }

        if (state.ball_position <= 0) {
            result.event = PlayEvent::DefensiveTouchdown;
            awardScoreAndKickoff(state, opponentOf(state.possession), 7, state.possession);
        } else if (state.ball_position >= 100) {
            result.event = PlayEvent::Touchback;
            touchback(state);
        } else {
            result.event = PlayEvent::Normal;
            turnoverAtSpot(state);
        }
        break;
    case OutcomeType::FieldGoalMade:
        result.event = PlayEvent::FieldGoalGood;
        result.turnover = TurnoverKind::None;
        awardScoreAndKickoff(state, state.possession, 3, opponentOf(state.possession));
        break;
    }

    state.time_remaining_seconds -= outcome.elapsed_seconds;

    if (state.time_remaining_seconds <= 0) {
        if (state.quarter == 2) {
            result.period = PeriodEvent::Halftime;
        } else {
            result.period = PeriodEvent::EndQuarter;
        }
        state.quarter++;
        state.time_remaining_seconds = 900;
        if (state.quarter == 3) {
            state.down = 1;
            state.distance = 10;
            state.ball_position = 25;
            state.possession = opponentOf(state.opening_possession);
        }
        if (state.quarter > 4) {
            result.period = PeriodEvent::EndofGame;
            state.quarter = 4;
            state.time_remaining_seconds = 0;
            state.game_over = true;
        }
    }
    return result;
}

PlayResult applyOutcome(const GameState& gameState, const PlayOutcome& playOutcome) {
    PlayResult result;
    result.state = gameState;

    const PlaySummary summary = applyOutcomeInPlace(result.state, playOutcome);
    result.event = summary.event;
    result.turnover = summary.turnover;
    result.period = summary.period;

    return result;
}
} // namespace gridiron
