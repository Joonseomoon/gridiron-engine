#pragma once

#include "GameState.h"
#include "PlayOutcome.h"

#include <optional>

namespace gridiron {

enum class PlayEvent {
    Normal,
    FirstDown,
    TouchDown,
    FieldGoalGood,
    Safety,
    Touchback,
    DefensiveTouchdown,
    PuntReturnTouchdown
};

enum class TurnoverKind { None, TurnoverOnDowns, FieldGoalMissed, Interception, Fumble };

enum class PeriodEvent { None, EndQuarter, Halftime, EndofGame, OvertimeStart };

struct PlayResult {
    GameState state;
    PlayEvent event = PlayEvent::Normal;
    TurnoverKind turnover = TurnoverKind::None;
    PeriodEvent period = PeriodEvent::None;
    std::optional<Possession> timeout_called = std::nullopt;
};

struct PlaySummary {
    PlayEvent event = PlayEvent::Normal;
    TurnoverKind turnover = TurnoverKind::None;
    PeriodEvent period = PeriodEvent::None;
};
} // namespace gridiron
