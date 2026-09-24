# Current round: M1.2 — Complete the game and switch areas

Goal: run complete seeded games end to end. Checkpoint requires checking impossible transitions and termination, not just that a game finishes.

## Shared prerequisite — do together, before splitting

**Give the sampler game context.** `samplePlayOutcome` / `choosePlayType` currently take only `(offense, defense, rng)`, so they cannot see down, distance, field position, or clock. Every task below depends on this signature, so agree it first or both halves will be built against a moving target.

Also agree who maps possession to profiles: `offense = (possession == Home) ? state.home : state.away`. Nothing owns this today — the M1.1 integration test passes the profiles in by hand, which hides the gap.

## Probability model

1. **4th-down field goal decision.** Per DECISIONS.md: on 4th down, if distance-to-goal <= 40, attempt a field goal; otherwise go for it. Belongs in `choosePlayType` once it can see `GameState`.
2. **Field goal make/miss sampling** from `field_goal_rating`, emitting `PlayType::FieldGoalAttempt` with `OutcomeType::FieldGoalMade` or `OutcomeType::Turnover` (miss). This is the first code to read `field_goal_rating`, and the first thing able to produce `FieldGoalAttempt` — both are currently unreachable in production.
3. **Checks of sampled behavior.** Over many samples, confirm the turnover rate and field goal success rate land near their configured values.

## Game progression

4. **Game loop.** Repeat sample → apply until `game_over`, using the possession-to-profile mapping above.
5. **Seeded opening coin toss** setting `opening_possession`. Already decided in DECISIONS.md, implemented nowhere — it currently sits at its `Home` default, and the halftime rule depends on it being real.
6. **Termination test.** Every play burns at least 10 seconds, so a game is bounded at roughly 360 plays. Assert it terminates rather than assuming it.

## Unassigned — either person

7. **Fix turnovers past either goal line.** See DECISIONS.md "Turnover past either goal line" for the agreed rule, and KNOWN_ISSUES.md for the failing tests. Currently `applyOutcome`'s `Turnover` case has no bounds guard, so it can produce a ball position of -5 or 105 and a negative `distance`. Must land before the checkpoint — "check impossible transitions" is exactly this.

## Checkpoint

- Run complete seeded games and inspect the play sequence together.
- Same seed, same teams reproduces an identical game.
- **Invariant sweep:** run many games across different seeds and assert every intermediate state is legal — down 1-4, distance >= 0, ball position 0-100, no 5th quarter, final scores reachable from 2/3/7-point plays. This is what would have caught the turnover bug.

## Deferred to M1.3

Readable event log and final result presentation, including `SimulationResult` and computing `Winner` (both defined and used by nothing today). See KNOWN_ISSUES.md — `applyOutcome` currently computes what happened on each play and discards it, which the log will need back.
