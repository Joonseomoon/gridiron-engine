#pragma once

#include "GameState.h"
#include "PlayOutcome.h"

namespace gridiron {

enum class PlayEvent { Normal, FirstDown, TouchDown, FieldGoalGood, Safety, Touchback, DefensiveTouchdown };

enum class TurnoverKind { None, TurnoverOnDowns, FieldGoalMissed, Interception, Fumble };

enum class PeriodEvent { None, EndQuarter, Halftime, EndofGame };

struct PlayResult {
    GameState state;
    PlayEvent event = PlayEvent::Normal;
    TurnoverKind turnover = TurnoverKind::None;
    PeriodEvent period = PeriodEvent::None;
};

} // namespace gridiron