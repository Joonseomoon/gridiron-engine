# Known Issues

Open problems found but not yet fixed. Remove entries as they are resolved.

## Turnover can put the ball outside 0-100 (rule agreed, fix scheduled in M1.2)

Agreed resolution is in DECISIONS.md under "Turnover past either goal line": touchback one way, defensive touchdown the other. Task 7 in TASKS.md.

`applyOutcome`'s `OutcomeType::Turnover` case applies `yards_gained` and flips the field with no bounds guard, unlike the `Normal` case which checks for `>= 100` (touchdown) and `< 0` (safety). Both directions break:

- Ball at 90, turnover gaining 15 → `105` → flipped to `100 - 105` = **-5**
- Ball at 5, turnover losing 10 → `-5` → flipped to `100 - (-5)` = **105**

Both violate acceptance criterion #3 ("Ball Position never outside 0-100"), and both are reachable from the real sampler, which can produce that yardage on a turnover play near either goal line. Existing turnover tests missed it because they all use modest yardage that stays in bounds.

The second case also corrupts `distance`: `turnoverAtSpot` computes `min(10, 100 - 105)` = **-5**. A negative distance means the next play converts a first down automatically, since `yards_gained >= distance` is true for almost any gain.

Failing tests: `ApplyOutcomeTest.TurnoverNearOpponentGoalLineStaysOnField`, `ApplyOutcomeTest.TurnoverBehindOwnGoalLineStaysOnField`. They assert the 0-100 invariant rather than a specific value, so they stay valid whichever fix we choose.

Fix options:
1. Clamp the turnover spot to `[0, 100]` before flipping. Two lines, no interface change.
2. Run the same goal-line/safety detection the `Normal` case uses before flipping, so a turnover past either goal line resolves as a score first.
3. Clip yardage to the field in the sampler — requires giving it `GameState` (the M1.2 interface change anyway).

Note the two directions are not symmetric: option 1 makes the second case *legal* but arguably models the wrong football outcome, since recovering a fumble in the opponent's end zone should be a defensive touchdown, not 1st & goal from inches. Needs a rules decision, not just a bounds patch.

## Event information is computed then discarded (deferred to M1.3)

`applyOutcome` works out whether a play scored, turned the ball over, or converted a first down, then returns only a `GameState`. The M1.3 event log will need that back, likely as a small result struct.

Diffing states is not enough: an interception, a turnover on downs, and a missed field goal all look like "possession flipped, score unchanged."

Deferred until there is a consumer. (`OutcomeType::Touchdown` and `Safety` were dropped from the enum — they are derived from field position, so no sampler could emit them. Those concepts belong in this output type instead.)
