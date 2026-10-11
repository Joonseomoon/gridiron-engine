#include "GameLog.h"
#include "../engine/GameState.h"
#include "../engine/PlayOutcome.h"
#include "../engine/PlayResult.h"
#include "../engine/SimulationResult.h"
#include <algorithm>
#include <iomanip>
#include <ostream>
#include <string>

namespace gridiron {

void printPlay(std::ostream& out, const GameState& before, const PlayOutcome& outcome, const PlayResult& result) {
    int play_time_consumed = outcome.elapsed_seconds;
    int yards_gained = outcome.yards_gained;
    // These are for before the play
    const std::string& offense = (before.possession == Possession::Home ? before.home.name : before.away.name);
    const std::string& defense = (before.possession == Possession::Home ? before.away.name : before.home.name);

    // These are for after the play (used for kickoffs)
    const std::string& afterOffense =
        (result.state.possession == Possession::Home ? result.state.home.name : result.state.away.name);

    std::string play_name = "";
    switch (outcome.play_type) {
    case PlayType::Run: play_name = "run"; break;
    case PlayType::Pass: play_name = "pass"; break;
    case PlayType::FieldGoalAttempt: play_name = "field goal attempt"; break;
    }

    std::string down = "";
    switch (result.state.down) {
    case 1: down = "1st"; break;
    case 2: down = "2nd"; break;
    case 3: down = "3rd"; break;
    case 4: down = "4th"; break;
    }

    const std::string& possessing_team =
        (result.state.possession == Possession::Home ? result.state.home.name : result.state.away.name);
    const std::string& opposing_team =
        (result.state.possession == Possession::Home ? result.state.away.name : result.state.home.name);
    std::string fieldPosition = "";
    if (result.state.ball_position > 50) {
        fieldPosition = opposing_team + " " + std::to_string(100 - result.state.ball_position);
    } else if (result.state.ball_position < 50) {
        fieldPosition = possessing_team + " " + std::to_string(result.state.ball_position);
    } else if (result.state.ball_position == 50) {
        fieldPosition = "midfield";
    }

    int seconds_left = std::max(0, before.time_remaining_seconds - play_time_consumed);
    int minutes = seconds_left / 60;
    int seconds = seconds_left % 60;
    int fieldGoalDistance = 100 - before.ball_position + 17; // Only for field goal position

    std::string distanceText = "";
    if (result.state.distance == 100 - result.state.ball_position) {
        distanceText = "goal";
    } else {
        distanceText = std::to_string(result.state.distance);
    }

    if (before.quarter == 1 && before.time_remaining_seconds == 900) {
        out << "Q1 15:00 - Game Start. " << offense << " receives the football.\n";
    }

    out << "Q" << before.quarter << " " << minutes << ":" << std::setfill('0') << std::setw(2) << seconds
        << std::setfill(' ') << " - ";

    if (result.event == PlayEvent::TouchDown) {
        out << offense << " " << play_name << " for " << yards_gained << " yards. Touchdown! " << offense << " +7.\n";
        if (result.period == PeriodEvent::None) {
            out << "Q" << result.state.quarter << " " << minutes << ":" << std::setfill('0') << std::setw(2) << seconds
                << std::setfill(' ') << " - " << afterOffense << " ball on " << afterOffense
                << " 25, 1st & 10 (kickoff).\n";
        }
    } else if (result.event == PlayEvent::DefensiveTouchdown) {
        if (result.turnover == TurnoverKind::Interception) {
            out << offense << " pass intercepted by " << defense << ". ";
        } else {
            out << offense << " fumble recovered by " << defense << ". ";
        }
        out << "Defensive touchdown! " << defense << " +7.\n";
        if (result.period == PeriodEvent::None) {
            out << "Q" << result.state.quarter << " " << minutes << ":" << std::setfill('0') << std::setw(2) << seconds
                << std::setfill(' ') << " - " << afterOffense << " ball on " << afterOffense
                << " 25, 1st & 10 (kickoff).\n";
        }
    } else if (result.turnover == TurnoverKind::Interception) {
        out << offense << " pass intercepted by " << defense << ". ";
        if (result.event == PlayEvent::Touchback) {
            out << "Touchback.\n";
            if (result.period == PeriodEvent::None) {
                out << "Q" << result.state.quarter << " " << minutes << ":" << std::setfill('0') << std::setw(2)
                    << seconds << std::setfill(' ') << " - " << afterOffense << " ball on " << afterOffense
                    << " 25, 1st & 10 (touchback).\n";
            }
        } else {
            out << "Turnover.\n";
            if (result.period == PeriodEvent::None) {
                out << "Q" << result.state.quarter << " " << minutes << ":" << std::setfill('0') << std::setw(2)
                    << seconds << std::setfill(' ') << " - " << afterOffense << " ball on " << fieldPosition
                    << ", 1st & " << distanceText << ".\n";
            }
        }
    } else if (result.turnover == TurnoverKind::Fumble) {
        out << offense << " fumble recovered by " << defense << ". ";
        if (result.event == PlayEvent::Touchback) {
            out << "Touchback.\n";
            if (result.period == PeriodEvent::None) {
                out << "Q" << result.state.quarter << " " << minutes << ":" << std::setfill('0') << std::setw(2)
                    << seconds << std::setfill(' ') << " - " << afterOffense << " ball on " << afterOffense
                    << " 25, 1st & 10 (touchback).\n";
            }
        } else {
            out << "Turnover.\n";
            if (result.period == PeriodEvent::None) {
                out << "Q" << result.state.quarter << " " << minutes << ":" << std::setfill('0') << std::setw(2)
                    << seconds << std::setfill(' ') << " - " << afterOffense << " ball on " << fieldPosition
                    << ", 1st & " << distanceText << ".\n";
            }
        }
    } else if (result.turnover == TurnoverKind::FieldGoalMissed) {
        out << offense << " field goal attempt from " << fieldGoalDistance << " yards: MISSED. Turnover.\n";
        if (result.period == PeriodEvent::None) {
            out << "Q" << result.state.quarter << " " << minutes << ":" << std::setfill('0') << std::setw(2) << seconds
                << std::setfill(' ') << " - " << afterOffense << " ball on " << fieldPosition << ", 1st & "
                << distanceText << ".\n";
        }
    } else if (result.turnover == TurnoverKind::TurnoverOnDowns) {
        out << offense << " " << play_name << " for " << yards_gained
            << " yards. Short of the first down. Turnover on downs.\n";
        if (result.period == PeriodEvent::None) {
            out << "Q" << result.state.quarter << " " << minutes << ":" << std::setfill('0') << std::setw(2) << seconds
                << std::setfill(' ') << " - " << afterOffense << " ball on " << fieldPosition << ", 1st & "
                << distanceText << ".\n";
        }
    } else if (result.event == PlayEvent::Safety) {
        out << offense << " " << play_name << " for " << yards_gained << " yards. Safety! " << defense << " +2.\n";
        if (result.period == PeriodEvent::None) {
            out << "Q" << result.state.quarter << " " << minutes << ":" << std::setfill('0') << std::setw(2) << seconds
                << std::setfill(' ') << " - " << afterOffense << " ball on " << afterOffense
                << " 25, 1st & 10 (kickoff).\n";
        }
    } else if (result.event == PlayEvent::FieldGoalGood) {
        out << offense << " field goal attempt from " << fieldGoalDistance << " yards: GOOD. " << offense << " +3.\n";
        if (result.period == PeriodEvent::None) {
            out << "Q" << result.state.quarter << " " << minutes << ":" << std::setfill('0') << std::setw(2) << seconds
                << std::setfill(' ') << " - " << afterOffense << " ball on " << afterOffense
                << " 25, 1st & 10 (kickoff).\n";
        }
    } else if (result.event == PlayEvent::FirstDown) {
        if (result.period == PeriodEvent::None) {
            out << offense << " " << play_name << " for " << yards_gained << " yards. First down! " << down << " & "
                << distanceText << " on " << fieldPosition << ".\n";
        } else {
            out << offense << " " << play_name << " for " << yards_gained << " yards. First down!\n";
        }
    } else if (result.event == PlayEvent::Normal) {
        if (result.period == PeriodEvent::None) {
            out << offense << " " << play_name << " for " << yards_gained << " yards. " << down << " & " << distanceText
                << " on " << fieldPosition << ".\n";
        } else {
            out << offense << " " << play_name << " for " << yards_gained << " yards.\n";
        }
    }

    if (result.period == PeriodEvent::EndQuarter) {
        out << "Q" << before.quarter << " " << minutes << ":" << std::setfill('0') << std::setw(2) << seconds
            << std::setfill(' ') << " - End of quarter.\n";
        out << "Q" << result.state.quarter << " 15:00 - " << afterOffense << " ball on " << fieldPosition << ", "
            << down << " & " << distanceText << ".\n";
    } else if (result.period == PeriodEvent::Halftime) {
        out << "Q" << before.quarter << " " << minutes << ":" << std::setfill('0') << std::setw(2) << seconds
            << std::setfill(' ') << " - Halftime.\n";
        out << "Q3 15:00 - " << afterOffense << " ball on " << afterOffense << " 25, 1st & 10 (second-half kickoff).\n";
    } else if (result.period == PeriodEvent::EndofGame) {
        out << "Q" << before.quarter << " " << minutes << ":" << std::setfill('0') << std::setw(2) << seconds
            << std::setfill(' ') << " - Game End.\n";
    }
}

void printFinalResult(std::ostream& out, const std::string& homeName, const std::string& awayName,
                      const SimulationResult& result) {

    std::string winOrTie = "";
    switch (result.winner) {
    case (Winner::Home): winOrTie = "Winner: " + homeName; break;
    case (Winner::Away): winOrTie = "Winner: " + awayName; break;
    case (Winner::Tie): winOrTie = "Winner: Tie"; break;
    }

    out << "Final Score\n";
    out << homeName << " - " << result.home_score << '\n';
    out << awayName << " - " << result.away_score << '\n';
    out << '\n';
    out << winOrTie << '\n';
    out << "Seed: " << result.seed << '\n';
}
} // namespace gridiron
