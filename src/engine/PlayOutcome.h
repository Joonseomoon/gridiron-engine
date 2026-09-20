#pragma once

namespace gridiron {

enum class PlayType {
    Run,
    Pass,
    Turnover,
    FieldGoalAttempt,
    Touchdown,
    Safety,
};

struct PlayOutcome {
    PlayType type;
    int yards_gained = 0;
    int elapsed_seconds = 0;
};

} // namespace gridiron
