#pragma once

namespace gridiron {

enum class PlayType {
    Run,
    Pass,
    Turnover,
    FieldGoalMade,
    FieldGoalMissed,
    Touchdown,
    Safety,
};

struct PlayOutcome {
    PlayType type;
    int yards_gained = 0;
    int elapsed_seconds = 0;
};

} // namespace gridiron
