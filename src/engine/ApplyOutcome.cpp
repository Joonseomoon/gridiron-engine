#include "ApplyOutcome.h"
#include "ModelConstants.h"
#include <algorithm>
#include <stdexcept>

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
    state.ball_position = kKickoffSpot;
    state.down = 1;
    state.distance = 10;
}

void changePossessionAtSpot(GameState& state) {
    state.possession = opponentOf(state.possession);
    state.ball_position = 100 - state.ball_position;
    state.down = 1;
    state.distance = std::min(10, 100 - state.ball_position);
}

void touchback(GameState& state, int spot) {
    state.possession = opponentOf(state.possession);
    state.ball_position = spot;
    state.down = 1;
    state.distance = 10;
}

bool clockStops(const GameState& before, const PlayOutcome& outcome) {
    const int afterPlay = before.time_remaining_seconds - outcome.elapsed_seconds;
    const bool q2 = before.quarter == 2;
    const bool q4 = before.quarter == 4;

    if (outcome.outcome_type == OutcomeType::Incomplete) {
        return true;
    }

    if (outcome.out_of_bounds &&
        ((q2 && afterPlay <= kTwoMinuteWarningSeconds) || (q4 && afterPlay <= kLateQ4OutOfBoundsSeconds))) {
        return true;
    }

    if ((q2 || q4) && before.time_remaining_seconds > kTwoMinuteWarningSeconds &&
        afterPlay <= kTwoMinuteWarningSeconds) {
        return true;
    }
    return false;
}

PlaySummary applyOutcomeInPlace(GameState& state, const PlayOutcome& outcome) {
    PlaySummary result;

    const bool stopped = clockStops(state, outcome);

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
            changePossessionAtSpot(state);
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
            touchback(state, kTurnoverTouchbackSpot);
        } else {
            result.event = PlayEvent::Normal;
            changePossessionAtSpot(state);
        }
        break;
    case OutcomeType::FieldGoalMade:
        result.event = PlayEvent::FieldGoalGood;
        result.turnover = TurnoverKind::None;
        awardScoreAndKickoff(state, state.possession, 3, opponentOf(state.possession));
        break;
    case OutcomeType::Incomplete:
        if (state.down < 4) {
            state.down++;
            result.event = PlayEvent::Normal;
            result.turnover = TurnoverKind::None;
        } else {
            result.turnover = TurnoverKind::TurnoverOnDowns;
            changePossessionAtSpot(state);
        }
        break;
    case OutcomeType::Punted:
        result.turnover = TurnoverKind::None;
        state.ball_position += outcome.yards_gained;
        if (state.ball_position >= 100) {
            result.event = PlayEvent::Touchback;
            touchback(state, kPuntTouchbackSpot);
        } else if (state.ball_position <= 0) {
            result.event = PlayEvent::PuntReturnTouchdown;
            awardScoreAndKickoff(state, opponentOf(state.possession), 7, state.possession);
        } else {
            result.event = PlayEvent::Normal;
            changePossessionAtSpot(state);
        }
        break;
    }

    state.time_remaining_seconds -= outcome.elapsed_seconds;
    if (!stopped) {
        if ((state.quarter == 2 || state.quarter == 4) && state.time_remaining_seconds > kTwoMinuteWarningSeconds &&
            state.time_remaining_seconds - outcome.runoff_seconds < kTwoMinuteWarningSeconds) {
            state.time_remaining_seconds = kTwoMinuteWarningSeconds;
        } else {
            state.time_remaining_seconds -= outcome.runoff_seconds;
        }
    }

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
            state.ball_position = kKickoffSpot;
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
