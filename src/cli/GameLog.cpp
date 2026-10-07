#include "GameLog.h"
#include "../engine/GameState.h"
#include "../engine/PlayOutcome.h"
#include "../engine/PlayResult.h"
#include <algorithm>
#include <iomanip>
#include <ostream>
#include <string>

namespace gridiron {

void printPlay(std::ostream& out, const GameState& before, const PlayOutcome& outcome, const PlayResult& result) {
    int play_time_consumed = outcome.elapsed_seconds;
    int yards_gained = outcome.yards_gained;
    const std::string& offense = (before.possession == Possession::Home ? before.home.name : before.away.name);
    const std::string& defense = (before.possession == Possession::Home ? before.away.name : before.home.name);

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
    std::string field_position = "";
    if (result.state.ball_position > 50) {
        field_position = opposing_team + " " + std::to_string(100 - result.state.ball_position);
    } else if (result.state.ball_position < 50) {
        field_position = possessing_team + " " + std::to_string(result.state.ball_position);
    } else if (result.state.ball_position == 50) {
        field_position = "midfield";
    }

    int seconds_left = std::max(0, before.time_remaining_seconds - play_time_consumed);
    int minutes = seconds_left / 60;
    int seconds = seconds_left % 60;

    out << "Q" << before.quarter << " " << minutes << ":" << std::setfill('0') << std::setw(2) << seconds
        << std::setfill(' ') << " - ";

    if (result.event == PlayEvent::TouchDown) {
        out << offense << " " << play_name << " for " << yards_gained << " yards. Touchdown! " << offense << "+7.\n";
    } else if (result.event == PlayEvent::DefensiveTouchdown) {
        if (result.turnover == TurnoverKind::Interception) {
            out << offense << " pass intercepted by" << defense << ". ";
        } else {
            out << offense << " fumble reovered by" << defense << ". ";
        }
        out << "Defensive touchdown! " << defense << " +7 \n.";
    }

    else if (result.turnover == TurnoverKind::Interception) {
        out << offense << " pass intercepted by " << defense << ". ";
        if (result.event == PlayEvent::Touchback) {
            out << "Touchback.\n";
        } else {
            out << "Turnover.\n";
        }
    } else if (result.turnover == TurnoverKind::Fumble) {
        out << offense << " fumble recovered by " << defense << ". ";
        if (result.event == PlayEvent::Touchback) {
            out << "Touchback.\n";
        } else {
            out << "Turnover.\n";
        }
    } else if (result.turnover == TurnoverKind::FieldGoalMissed) {
        out << offense << " field goal attempt from " <<
    }

    else if (result.event == PlayEvent::Normal) {
        out << offense << " " << play_name << " for " << yards_gained << " yards. " << down << " & "
            << result.state.distance << " on " << field_position << ".\n";
    } else if (result.event == PlayEvent::FirstD)
        << offense << " " << play_name << " for " << yards_gained << " yards. " << down << " & "
        << result.state.distance << " on " << field_position << ".\n";

} // namespace gridiron