# Side tasks

Work that is not in the ROADMAP.md tables: bugs found along the way, tooling, and deliberately deferred tests. Roadmap
work is tracked in ROADMAP.md and its issues, not here. Remove an item once it is merged.

## Bugs

## Deferred

### Kickoff and turnover touchback spots (4.1 checkpoint)

Implemented in the unmerged M4 integration work: kickoffs use 35, turnover touchbacks use 20, and punt
touchbacks remain at 20. Opening/halftime/overtime starts and logs agree with these spots. Updated with the
`m4-v1` tuning and fingerprints in `decisions/M4.md`. Remove this item after the integration PR merges.

### clang-tidy

Static analysis beyond compiler warnings and sanitizers; it could also mechanically enforce one naming convention (
locals currently mix camelCase and snake_case). Noisy to set up, so it waits until M3, when performance work and
threading make it pay off. Start with a narrow check set (`bugprone-*`, `performance-*`) rather than everything.
