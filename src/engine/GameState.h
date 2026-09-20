#pragma once

#include "TeamProfile.h"

namespace gridiron {

enum class Possession { Home, Away };

struct GameState {
    TeamProfile home;
    TeamProfile away;

    int quarter = 1;
    int time_remaining_seconds = 15 * 60;

    Possession opening_possession = Possession::Home;
    Possession possession = Possession::Home;
    int down = 1;
    int distance = 10;
    // Yards from the possessing team's own goal line, 0-100.
    int ball_position = 25;

    int home_score = 0;
    int away_score = 0;

    bool game_over = false;
};

} // namespace gridiron
