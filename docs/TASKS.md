# Current round: M1.2 — Complete the game and switch areas

Goal: run complete seeded games end to end. Checkpoint requires checking impossible transitions and termination, not just that a game finishes.

## Shared prerequisite — do together, before splitting

**Give the sampler game context.** `samplePlayOutcome` / `choosePlayType` currently take only `(offense, defense, rng)`, so they cannot see down, distance, field position, or clock. Every task below depends on this signature, so agree it first or both halves will be built against a moving target.

Also agree who maps possession to profiles: `offense = (possession == Home) ? state.home : state.away`. Nothing owns this today — the M1.1 integration test passes the profiles in by hand, which hides the gap.

## Probability model

1. ~~**4th-down field goal decision.**~~ Done. `choosePlayType` now takes `GameState` and returns `FieldGoalAttempt` on 4th down with distance-to-goal <= 40, with tests pinning both clauses of that condition independently and the boundary at exactly 40.
2. ~~**Field goal make/miss sampling.**~~ Done. Distance-dependent, per the `field_goal_rating` definition recorded in DECISIONS.md. `samplePlayOutcome` now derives offense/defense from `possession` itself, so the shared prerequisite is settled: the mapping lives in the sampler, not in callers.
3. ~~**Checks of sampled behavior.**~~ Done, and extended well past the original scope after mutation testing showed the sampler suite caught only 1 of 18 planted bugs. It now covers the yardage formulas (mean, spread, rating direction, which ratings feed which formula), turnover frequency at a non-trivial rate, the over-20-yard duration branch, the run/pass split, and dispatch-matches-decision.

Still uncaught by design: a change to the *order* random numbers are drawn. Only a golden test with hard-coded sampled values catches that, and those values are implementation-defined for `std::normal_distribution` — adding one would pin CI to macOS/libc++. Deliberately deferred rather than forgotten.

## Game progression

4. ~~**Game loop.**~~ Done. `runGame` in `GameRunner` repeats sample → apply until `game_over`, and throws if a game passes the 360-play bound instead of looping forever.
5. ~~**Seeded opening coin toss**~~ Done. Drawn first from the game's RNG stream, so the same seed reproduces it.
6. ~~**Termination test.**~~ Done, including extreme team profiles and turnover rates of 0 and 1.

## Also on the probability model side

7. ~~**Fix turnovers past either goal line.**~~ Done. Touchback and defensive touchdown implemented per DECISIONS.md "Turnover past either goal line", with boundary tests at exactly 0 and exactly 100 and a defensive-touchdown-at-halftime test.

## Checkpoint

Done in `tests/invariant_sweep_test.cpp`. `runGame` now has an overload taking a per-play observer `(before, outcome, after)` so the sweep can see every intermediate state; it draws no random numbers, and a test checks that a seed produces the same game with or without one. M1.3's event log can use the same hook. The sweep runs 1000 seeds × 5 matchups (typical, both extreme mismatches, turnover-heavy, and a defensive struggle that ends most drives on 4th down) and checks the bounds below plus: the field goal decision rule on every snap, one team scoring per play by 2/3/7 only, final score equal to the sum of scoring plays, clock monotonicity, the post-score kickoff spot, and the halftime possession rule. Planting the old turnover-past-the-goal-line bug, or handing the second half to the opening team, makes it fail.

- Run complete seeded games and inspect the play sequence together.
- Same seed, same teams reproduces an identical game.
- **Invariant sweep:** run many games across different seeds and assert every intermediate state is legal — down 1-4, distance >= 0, ball position **1-99**, no 5th quarter, final scores reachable from 2/3/7-point plays. This is what would have caught the turnover bug. Note 1-99, not 0-100: at or past either goal line now always resolves as a score or a touchback, so a live ball never rests on 0 or 100 (see DECISIONS.md "Goal line boundaries").

## At the round boundary, once nobody has open work

**Set up clang-format.** Formatting is currently inconsistent between files (brace placement, spacing), and locals mix camelCase with snake_case while struct members are snake_case. A formatter ends that class of review comment for good.

Do it only when neither of us has an open branch — reformatting every file while someone has in-flight work means merge conflicts in every file they touched, for no functional reason. Then:

- Make it its own commit, touching nothing but formatting.
- Add `.git-blame-ignore-revs` with that commit's SHA so `git blame` still points at whoever wrote each line.
- Enforce it in CI (`clang-format --dry-run --Werror`), or it drifts back within a few PRs.

Good fit for Developer B: LEARNING.md shows build/tests/docs as A-only so far, and CONTRIBUTING.md says to fix areas with a single implementer.

Optional follow-on: clang-tidy, which can mechanically enforce the naming convention. Noisier, so it can wait. Note neither tool would have caught any of the bugs found this round — they were all football-rules reasoning. This is for clean diffs, not correctness.

## Deferred to M1.3

Readable event log and final result presentation, including `SimulationResult` and computing `Winner` (both defined and used by nothing today).

`applyOutcome` works out whether a play scored, turned the ball over, or converted a first down, then returns only a `GameState` — discarding it. The log will need that back, likely as a small result struct rather than a bare `GameState`.

Diffing before/after states is not enough: an interception, a turnover on downs, and a missed field goal all look like "possession flipped, score unchanged," but they are three different sentences in a log. (`OutcomeType::Touchdown` and `Safety` were dropped from the sampler's enum for the same reason — they are derived from field position, so no sampler could emit them. Those concepts belong in this output type instead.)

### Developer A — fix the halftime kickoff rule

The second half always starts with the team that did **not** open the game receiving at its own 25, 1st and 10 — even when the last play of the first half scored. This is the rule DECISIONS.md "End of quarter" already states (and the real NFL rule); the code currently disagrees.

- In `applyOutcome`, drop the `!wasScoringPlay` condition on the quarter-3 reset so halftime always overrides the scoring play's kickoff. The points from that play still count.
- `PassCrossesGoalLineOnFinalPlayOfHalfStillScores` and `SafetyToEndHalfKeepsItsOwnKickoffWhenAwayOpened` assert the old behavior; rewrite them to expect the opponent of `opening_possession` to receive.
- In `tests/invariant_sweep_test.cpp`, remove the `&& !scored` exemption from the halftime check so the sweep enforces the rule on every game.
