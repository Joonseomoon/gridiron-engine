#pragma once

namespace gridiron {

enum class PlayType { Run, Pass, FieldGoalAttempt, Punt };

enum class OutcomeType { Normal, Turnover, FieldGoalMade, Incomplete, Punted };

struct PlayOutcome {
    PlayType play_type;
    OutcomeType outcome_type;
    int yards_gained = 0;
    int elapsed_seconds = 0;
    int runoff_seconds = 0;
    bool out_of_bounds = false;
};

} // namespace gridiron
