#pragma once

namespace gridiron {

enum class PlayType {
    Run,
    Pass,
    FieldGoalAttempt
};

enum class OutcomeType {
    Normal,
    Turnover,
    Touchdown,
    Safety
};
struct PlayOutcome {
    PlayType play_type;
    OutcomeType outcome_type;
    int yards_gained = 0;
    int elapsed_seconds = 0;
};

} // namespace gridiron
