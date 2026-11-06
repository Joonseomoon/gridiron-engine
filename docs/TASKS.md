# Side tasks

Work that is not in the ROADMAP.md tables: bugs found along the way, tooling, and deliberately deferred tests. Roadmap
work is tracked in ROADMAP.md and its issues, not here. Remove an item once it is merged.

## Bugs

## Deferred

### Teams out of `GameState` (M4)

`GameState` holds both `TeamProfile`s, so every copy carries team data that never changes during a game. M4 resumes
games from a supplied situation and simulates from it many times, so decide then whether `GameState` should hold only
the situation (score, clock, possession, down, distance, field position) with teams passed separately. It touches the
samplers, `GameLog`, and every test that builds a `GameState` with teams. Measured context: M3 decisions, per-play copy.

### clang-tidy

Static analysis beyond compiler warnings and sanitizers; it could also mechanically enforce one naming convention (
locals currently mix camelCase and snake_case). Noisy to set up, so it waits until M3, when performance work and
threading make it pay off. Start with a narrow check set (`bugprone-*`, `performance-*`) rather than everything.
