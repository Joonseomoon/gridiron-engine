#pragma once

namespace gridiron {

enum class Possession { Home, Away };

struct GameState {
    int quarter = 1;
    int time_remaining_seconds = 15 * 60;

    int home_timeouts = 3;
    int away_timeouts = 3;

    Possession opening_possession = Possession::Home;
    Possession possession = Possession::Home;
    int down = 1;
    int distance = 10;
    // Yards from the possessing team's own goal line, 0-100.
    int ball_position = 25;

    int home_score = 0;
    int away_score = 0;

    bool game_over = false;

    int overtime_possessions_completed = 0;
};

} // namespace gridiron
