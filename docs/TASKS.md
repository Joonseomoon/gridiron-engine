# Side tasks

Work that is not in the ROADMAP.md tables: bugs found along the way, tooling, and deliberately deferred tests. Roadmap
work is tracked in ROADMAP.md and its issues, not here. Remove an item once it is merged.

## Bugs

## Deferred

### Kickoff and turnover touchback spots (4.1 checkpoint)

Scores place the receiving team at the 25 (`kKickoffSpot`) and turnover touchbacks also use the 25
(`kTurnoverTouchbackSpot`). The NFL kickoff touchback is the 35 (since 2025) and a turnover touchback is the 20. Punts
already use the 20. Changing either changes output, so it waits for the 4.1 checkpoint's retune and `m4-v1` bump; the
kickoff change raises scoring slightly, so make it before tuning totals.

### clang-tidy

Static analysis beyond compiler warnings and sanitizers; it could also mechanically enforce one naming convention (
locals currently mix camelCase and snake_case). Noisy to set up, so it waits until M3, when performance work and
threading make it pay off. Start with a narrow check set (`bugprone-*`, `performance-*`) rather than everything.
