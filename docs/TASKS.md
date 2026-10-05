# Side tasks

Work that is not in the ROADMAP.md tables: bugs found along the way, tooling, and deliberately deferred tests. Roadmap
work is tracked in ROADMAP.md and its issues, not here. Remove an item once it is merged.

## Bugs

### Halftime kickoff rule (Developer A, M1.3)

The second half always starts with the team that did **not** open the game receiving at its own 25, 1st and 10 — even
when the last play of the first half scored. This is the rule DECISIONS.md "End of quarter" already states (and the real
NFL rule); the code currently disagrees.

- In `applyOutcome`, drop the `!wasScoringPlay` condition on the quarter-3 reset so halftime always overrides the
  scoring play's kickoff. The points from that play still count.
- `PassCrossesGoalLineOnFinalPlayOfHalfStillScores` and `SafetyToEndHalfKeepsItsOwnKickoffWhenAwayOpened` assert the old
  behavior; rewrite them to expect the opponent of `opening_possession` to receive.
- In `tests/invariant_sweep_test.cpp`, remove the `&& !scored` exemption from the halftime check so the sweep enforces
  the rule on every game.

## Deferred

### clang-tidy

Static analysis beyond compiler warnings and sanitizers; it could also mechanically enforce one naming convention (
locals currently mix camelCase and snake_case). Noisy to set up, so it waits until M3, when performance work and
threading make it pay off. Start with a narrow check set (`bugprone-*`, `performance-*`) rather than everything.
