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

    void awardScoreAndKickoff(GameState& state, const Possession scoringTeam, const int points, const Possession receivingTeam) {
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

    GameState applyOutcome(const GameState& gameState, const PlayOutcome& playOutcome) {
        GameState newState = gameState;

        bool wasScoringPlay = false;

        switch (playOutcome.outcome_type) {
            case OutcomeType::Normal:
                newState.ball_position += playOutcome.yards_gained;
                if (newState.ball_position >= 100) {
                    // A gain that reaches or crosses the goal line scores,
                    // regardless of down/distance.
                    awardScoreAndKickoff(newState, newState.possession, 7, opponentOf(newState.possession));
                    wasScoringPlay = true;
                } else if (newState.ball_position <= 0) {
                    // Tackled in the offense's own end zone: safety.
                    awardScoreAndKickoff(newState, opponentOf(newState.possession), 2, opponentOf(newState.possession));
                    wasScoringPlay = true;
                } else if (playOutcome.yards_gained >= newState.distance) {
                    // First down conversion
                    newState.distance = std::min(10, 100 - newState.ball_position);
                    newState.down = 1;
                } else if (newState.down == 4 && playOutcome.yards_gained < newState.distance) {
                    turnoverAtSpot(newState);
                } else {
                    newState.distance -= playOutcome.yards_gained;
                    newState.down++;
                }
                break;
            case OutcomeType::Turnover:
                if (playOutcome.play_type != PlayType::FieldGoalAttempt) {
                    newState.ball_position += playOutcome.yards_gained;
                }

                if (newState.ball_position <= 0) {
                    awardScoreAndKickoff(newState, opponentOf(newState.possession), 7, newState.possession);
                    wasScoringPlay = true;
                } else if (newState.ball_position >= 100) {
                    touchback(newState);
                } else {
                    turnoverAtSpot(newState);
                }
                break;
            case OutcomeType::FieldGoalMade:
                awardScoreAndKickoff(newState, newState.possession, 3, opponentOf(newState.possession));
                wasScoringPlay = true;
                break;
        }

        newState.time_remaining_seconds -= playOutcome.elapsed_seconds;
        if (newState.time_remaining_seconds <= 0) {
            newState.quarter++;
            newState.time_remaining_seconds = 900;
            if (newState.quarter == 3 && !wasScoringPlay) {
                newState.down = 1;
                newState.distance = 10;
                newState.ball_position = 25;
                newState.possession = opponentOf(newState.opening_possession);
            }
            if (newState.quarter > 4) {
                newState.quarter = 4;
                newState.time_remaining_seconds = 0;
                newState.game_over = true;
            }
        }
        return newState;
    }
}
