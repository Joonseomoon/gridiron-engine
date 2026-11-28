#include "cli/GameLog.h"
#include "engine/batch/SimulationResult.h"
#include "engine/core/GameState.h"
#include "engine/core/PlayOutcome.h"
#include "engine/core/PlayResult.h"
#include <iomanip>
#include <ostream>
#include <string>

namespace gridiron {
namespace {
const std::string& teamName(const Matchup& matchup, Possession side) {
    return side == Possession::Home ? matchup.home.name : matchup.away.name;
}

Possession other(Possession side) {
    return side == Possession::Home ? Possession::Away : Possession::Home;
}
} // namespace

void printPlay(std::ostream& out, const GameState& before, const Matchup& matchup, const PlayOutcome& outcome,
               const PlayResult& result) {
    int seconds_left = before.quarter == result.state.quarter ? result.state.time_remaining_seconds : 0;
    int yards_gained = outcome.yards_gained;
    // These are for before the play
    const std::string& offense = teamName(matchup, before.possession);
    const std::string& defense = teamName(matchup, other(before.possession));

    // These are for after the play (used for kickoffs)
    const std::string& afterOffense = teamName(matchup, result.state.possession);

    std::string play_name = "";
    switch (outcome.play_type) {
    case PlayType::Run: play_name = "run"; break;
    case PlayType::Pass: play_name = "pass"; break;
    case PlayType::FieldGoalAttempt: play_name = "field goal attempt"; break;
    case PlayType::Punt: play_name = "punt"; break;
    }

    // "BAL pass incomplete" or "BAL run for 6 yards"
    const bool incomplete = outcome.outcome_type == OutcomeType::Incomplete;
    const std::string playText = incomplete
                                     ? offense + " pass incomplete"
                                     : offense + " " + play_name + " for " + std::to_string(yards_gained) + " yards";

    std::string down = "";
    switch (result.state.down) {
    case 1: down = "1st"; break;
    case 2: down = "2nd"; break;
    case 3: down = "3rd"; break;
    case 4: down = "4th"; break;
    }

    const std::string& possessing_team = teamName(matchup, result.state.possession);
    const std::string& opposing_team = teamName(matchup, other(result.state.possession));
    std::string fieldPosition = "";
    if (result.state.ball_position > 50) {
        fieldPosition = opposing_team + " " + std::to_string(100 - result.state.ball_position);
    } else if (result.state.ball_position < 50) {
        fieldPosition = possessing_team + " " + std::to_string(result.state.ball_position);
    } else if (result.state.ball_position == 50) {
        fieldPosition = "midfield";
    }

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

    if (outcome.play_type == PlayType::Punt) {
        std::string how = "";
        if (result.event == PlayEvent::PuntReturnTouchdown) {
            out << offense << " punt returned for a touchdown by " << defense << "! " << defense << " +7.\n";
            how = " (kickoff)";
        } else if (result.event == PlayEvent::Touchback) {
            out << offense << " punts into the end zone. Touchback.\n";
            how = " (touchback)";
        } else {
            out << offense << " punts " << yards_gained << " yards net.\n";
        }
        if (result.period == PeriodEvent::None) {
            out << "Q" << result.state.quarter << " " << minutes << ":" << std::setfill('0') << std::setw(2) << seconds
                << std::setfill(' ') << " - " << afterOffense << " ball on " << fieldPosition << ", 1st & "
                << distanceText << how << ".\n";
        }
    } else if (result.event == PlayEvent::TouchDown) {
        out << offense << " " << play_name << " for " << yards_gained << " yards. Touchdown! " << offense << " +7.\n";
        if (result.period == PeriodEvent::None) {
            out << "Q" << result.state.quarter << " " << minutes << ":" << std::setfill('0') << std::setw(2) << seconds
                << std::setfill(' ') << " - " << afterOffense << " ball on " << afterOffense << " "
                << result.state.ball_position << ", 1st & 10 (kickoff).\n";
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
                << std::setfill(' ') << " - " << afterOffense << " ball on " << afterOffense << " "
                << result.state.ball_position << ", 1st & 10 (kickoff).\n";
        }
    } else if (result.turnover == TurnoverKind::Interception) {
        out << offense << " pass intercepted by " << defense << ". ";
        if (result.event == PlayEvent::Touchback) {
            out << "Touchback.\n";
            if (result.period == PeriodEvent::None) {
                out << "Q" << result.state.quarter << " " << minutes << ":" << std::setfill('0') << std::setw(2)
                    << seconds << std::setfill(' ') << " - " << afterOffense << " ball on " << afterOffense << " "
                    << result.state.ball_position << ", 1st & 10 (touchback).\n";
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
                    << seconds << std::setfill(' ') << " - " << afterOffense << " ball on " << afterOffense << " "
                    << result.state.ball_position << ", 1st & 10 (touchback).\n";
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
        out << playText << (incomplete ? "" : ". Short of the first down") << ". Turnover on downs.\n";
        if (result.period == PeriodEvent::None) {
            out << "Q" << result.state.quarter << " " << minutes << ":" << std::setfill('0') << std::setw(2) << seconds
                << std::setfill(' ') << " - " << afterOffense << " ball on " << fieldPosition << ", 1st & "
                << distanceText << ".\n";
        }
    } else if (result.event == PlayEvent::Safety) {
        out << offense << " " << play_name << " for " << yards_gained << " yards. Safety! " << defense << " +2.\n";
        if (result.period == PeriodEvent::None) {
            out << "Q" << result.state.quarter << " " << minutes << ":" << std::setfill('0') << std::setw(2) << seconds
                << std::setfill(' ') << " - " << afterOffense << " ball on " << afterOffense << " "
                << result.state.ball_position << ", 1st & 10 (kickoff).\n";
        }
    } else if (result.event == PlayEvent::FieldGoalGood) {
        out << offense << " field goal attempt from " << fieldGoalDistance << " yards: GOOD. " << offense << " +3.\n";
        if (result.period == PeriodEvent::None) {
            out << "Q" << result.state.quarter << " " << minutes << ":" << std::setfill('0') << std::setw(2) << seconds
                << std::setfill(' ') << " - " << afterOffense << " ball on " << afterOffense << " "
                << result.state.ball_position << ", 1st & 10 (kickoff).\n";
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
            out << playText << ". " << down << " & " << distanceText << " on " << fieldPosition << ".\n";
        } else {
            out << playText << ".\n";
        }
    }
    if (result.timeout_called.has_value()) {
        const Possession caller = *result.timeout_called;
        const int remaining = caller == Possession::Home ? result.state.home_timeouts : result.state.away_timeouts;

        out << "Q" << before.quarter << " " << minutes << ":" << std::setfill('0') << std::setw(2) << seconds
            << std::setfill(' ') << " - " << teamName(matchup, caller) << " timeout. " << remaining << " remaining.\n";
    }
    if (result.period == PeriodEvent::EndQuarter) {
        out << "Q" << before.quarter << " " << minutes << ":" << std::setfill('0') << std::setw(2) << seconds
            << std::setfill(' ') << " - End of quarter.\n";
        out << "Q" << result.state.quarter << " 15:00 - " << afterOffense << " ball on " << fieldPosition << ", "
            << down << " & " << distanceText << ".\n";
    } else if (result.period == PeriodEvent::Halftime) {
        out << "Q" << before.quarter << " " << minutes << ":" << std::setfill('0') << std::setw(2) << seconds
            << std::setfill(' ') << " - Halftime.\n";
        out << "Q3 15:00 - " << afterOffense << " ball on " << afterOffense << " " << result.state.ball_position
            << ", 1st & 10 (second-half kickoff).\n";
    } else if (result.period == PeriodEvent::OvertimeStart) {
        out << "Q4 0:00 - Regulation ends tied.\n";
        out << "Q5 10:00 - Overtime begins. " << afterOffense << " receives the football. "
            << "Each team has 2 timeouts.\n";
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
