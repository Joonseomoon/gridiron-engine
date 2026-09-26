# Initial Decisions

Fill this in together during Milestone 0. Keep decisions short and revise them as evidence changes.

## Already agreed

- Build incrementally around a reusable C++ simulation engine.
- Start with hand-set ratings and documented rule simplifications.
- Both developers build connected functionality and own its tests.
- Implementation areas rotate within milestones; most task work proceeds in parallel after shared prerequisites.
- Integrate early, review each other's work, and keep main working.
- Historical market analysis comes after model evaluation.

## Decide before V1

- Supported development operating systems:
    - macOS (both developers currently on macOS). Not yet verified on Linux/Windows; revisit if either becomes needed (e.g. CI or a future collaborator).

- C++ standard and compiler baseline:
    - C++20 (already set in CMakeLists.txt via CMAKE_CXX_STANDARD 20).
    - Apple Clang 14+ (Xcode 14+ command line tools). GCC 11+/Clang 14+ would also work if Linux support is added later.

- Build tool:
    - CMake (3.20+, already set up in CMakeLists.txt). Sufficient for this project's scope long-term: multiple targets (engine library, CLI, tests, later batch runner/API) without switching tools; FetchContent covers adding a test framework (e.g. GoogleTest/Catch2) or other small dependencies when needed.

- Team input fields:
    - Team Name
    - Offensive Pass Rating
    - Offensive Run Rating
    - Defensive Pass Rating
    - Defensive Run Rating
    - Turnover Rate (Offense)
    - Field Goal Rating

- Game state fields:
    - Quarter (1-4; overtime out of scope for V1)
    - Time Remaining in Quarter (seconds)
    - Possession (which team has the ball)
    - Down (1-4)
    - Distance (yards needed for a first down)
    - Ball Position (yard line, 0-100, expressed as distance from possessing team's own goal line)
    - Home Team Score
    - Away Team Score
    - Game Over (bool)

- Simulation result fields:
    - Home Team Score
    - Away Team Score
    - Winner (Can be Tie)
    - Seed

    -Example: BAL vs SEA, 
             Final Score
             BAL - 27
             SEA - 21

             Winner: BAL
             Seed: 1000

- Play-level or drive-level initial simulation:
    - Play-level. Each simulated play is a pass or run attempt sampled from the possessing team's offensive rating vs. the opponent's defensive rating, producing a yardage gain (or turnover/score) that updates the game state. Drive-level aggregation can be layered on top later; it should not be the unit the engine reasons about.

- Clock and game-end simplifications:
    - Four 15-minute quarters (900 simulated seconds each), running clock only — no explicit play clock, no timeouts, no two-minute warning in V1.
    - Each play consumes a sampled amount of game time (e.g. ~30-40 seconds), decremented from Time Remaining in Quarter.
    - Quarter increments when time reaches 0; halftime is not modeled as a separate state, just a boundary between Q2 and Q3.
    - Game ends when Quarter 4 time reaches 0. Ties are allowed in V1 (no overtime) — overtime is deferred to a later milestone per DECISIONS "Special teams... simplifications" below.

- Downs, turnovers, scoring, and possession rules:
    - Standard 4 downs to gain 10 yards for a first down; failing to convert on 4th down turns the ball over on downs at the current spot.
    - Turnovers (interception/fumble, sampled via each team's Turnover Rate) immediately flip possession at the spot of the turnover; down resets to 1st and distance to 10.
    - Touchdown when Ball Position reaches the opponent's goal line (yard 100 from the scoring team's perspective): +6 points, followed by an automatic extra point (+1), no 2-point attempts in V1.
    - Field goal decision on 4th down is a fixed rule: if the ball is within 40 yards of the opponent's goal line (i.e. distance-to-goal <= 40), attempt a field goal; otherwise, go for it on 4th down. Make = +3 and possession flips (kickoff simplification below), miss = turnover on downs at the spot.
    - Field goal success depends on both Field Goal Rating and kick distance, where kick distance = distance-to-goal + 17 (10 yards of end zone, 7 yards of snap and hold). So the 40-yard cutoff above allows attempts up to roughly 57 yards. Longer kicks are less likely to convert; the exact curve is a hand-tuned placeholder to be replaced with fitted data in M5, like the yardage constants.
    - Safety (offense tackled behind their own goal line): +2 to the defense, possession flips to that defense via a simplified kickoff.
    - After any score, possession flips via a fixed-field-position kickoff simplification (e.g. receiving team starts at their own 25) rather than simulating the kick itself.

- Special teams, penalties, ties, and overtime simplifications:
    - No penalties in V1.
    - No punts in V1 — 4th-down decisions collapse to "go for it" or "attempt field goal" per the FG-attempt heuristic above; punting is a documented simplification to add in a later round.
    - Kickoffs are not simulated as plays; possession after scores/halftime starts at a fixed yard line (see above).
    - No overtime in V1 — games can end in a tie. Overtime is explicitly out of scope until a later milestone.

- Fixed-seed reproducibility scope:
    - A single integer seed initializes one RNG stream for the entire game (all play-outcome sampling: pass/run yardage, turnovers, field goal attempts). The same seed with the same two TeamProfiles must always reproduce an identical play-by-play log and final score.

- First terminal demo and acceptance criteria:
    - Command: `simulate BAL SEA --seed 1000`

    - Sample terminal interaction (illustrative excerpt, not a measured result):
      ```
      $ simulate BAL SEA --seed 1000
      Q1 15:00 - BAL ball on BAL 25, 1st & 10
      Q1 14:32 - BAL run for 6 yards. 2nd & 4 on BAL 31.
      Q1 13:58 - BAL pass complete for 12 yards. 1st & 10 on BAL 43.
      Q1 13:20 - BAL pass intercepted by SEA at SEA 48. Turnover.
      Q1 13:20 - SEA ball on SEA 48, 1st & 10
      ...
      Q1 09:47 - BAL run for 3 yards. 4th & 1 on SEA 8.
      Q1 09:12 - BAL field goal attempt from SEA 8: GOOD. BAL +3.
      Q1 09:12 - SEA ball on SEA 25, 1st & 10 (kickoff)
      ...
      Q4 00:00 - End of game.

      Final Score
      BAL - 27
      SEA - 21

      Winner: BAL
      Seed: 1000
      ```

    - Acceptance criteria:
        1. **Determinism:** running `simulate BAL SEA --seed 1000` twice produces byte-identical output (full log and final score), given unchanged TeamProfiles.
        2. **Seed sensitivity:** running with a different seed (same teams) can produce a different log and/or final score; it is not required to differ, but the engine must not silently ignore the seed.
        3. **Legal state only:** the log never shows a down outside 1-4, a negative Distance, or a Ball Position outside 0-100.
        4. **Score consistency:** `Home Team Score` and `Away Team Score` are each expressible as a non-negative combination of scoring plays worth 2, 3, 6, or 7 points (safety, field goal, touchdown-no-XP, touchdown-with-XP); the printed final score matches the sum of scoring events shown in the log.
        5. **Termination:** the simulation always ends by the end of Q4 (`Game Over` becomes true); it never loops indefinitely and never simulates a 5th quarter.
        6. **Winner correctness:** `Winner` matches whichever team has the higher final score, or is `Tie` when scores are equal.
        7. **Rule fidelity:** the log is inspectable enough to manually verify against the agreed rules above — e.g. a fan reading the log can confirm a field goal was only attempted on 4th down with distance-to-goal <= 40, and that a turnover immediately reset the down/distance and flipped possession.
        8. **Fresh-checkout run:** a developer who has not touched the code can build and run the exact command above from a clean checkout and get output matching this format, with no manual setup beyond the documented build steps.

Prefer play-level simulation if both developers are comfortable with its scope. If choosing drive-level initially, plan to add down-and-distance state before strategy analysis.


## M1.1 Sampling assumptions (as implemented)

These are modeling choices baked into OutcomeSampler that were not decided in M0. Recorded here so they are deliberate and reviewable, not accidental. Revise as evidence changes.

- **Run/pass split is a fixed 50/50 coin flip**, independent of team ratings, down, distance, field position, or clock. Situational play calling is out of scope for V1.
- **Yardage is sampled from a normal distribution** around `base + k * (offense rating - defense rating)` — base 2 yards / k=10 / sd 3.0 for runs, base 4 yards / k=15 / sd 5.0 for passes. These constants are hand-tuned guesses, not fitted to data; replacing them with real estimates is the point of M5.
- **Yardage is never clipped to the field.** A sampled gain may push the ball past 100 or below 0; applyOutcome converts those into a touchdown or safety respectively. This is intended behavior, not an accident — the sampler stays ignorant of field geometry.
- **Turnover yardage is the yardage the play would have gained.** On a turnover, the sampler reports the same `yards_gained` it would have on a normal play, and applyOutcome advances the ball that far before flipping possession. This stands in for "the spot of the turnover" without modeling returns separately.
- **Turnover probability is a flat per-play rate** (`offense.turnover_rate`), identical for runs and passes, and unaffected by situation.
- **Play duration is sampled independently of the play's result**: ~N(30, 5) with a 15s floor for gains over 20 yards, ~N(20, 5) with a 10s floor otherwise. The clock does not distinguish in-bounds from out-of-bounds or incomplete passes, per the running-clock simplification above.
- **A field goal attempt burns the same clock as an ordinary short play** (~N(20, 5), 10s floor). Realistically a snap-kick-whistle is quicker, but reusing the existing distribution keeps one source of play timing and preserves the "every play burns at least 10 seconds" bound that game termination relies on.
- **`field_goal_rating` means make probability on a 20-yard chip shot.** Success is `rating - 0.01 * (kickDistance - 20)`, clamped to [0.01, 0.99], where `kickDistance = distance-to-goal + 17`. So a realistic kicker rates about 0.95, not 0.8: at 0.95 that gives roughly 0.95 at a chip shot, 0.79 from 40 yards, and 0.58 from the 57-yard edge of the attempt range. Hand-tuned like the yardage constants, to be refit in M5.

## M0.2 State-transition cases

### Normal pass/run play
- Sample play resylt using the possessing team's offese pass/run rating and opponent's pass/run deafense rating
- Subtract the sampled play time from the time remaining in the quarter
- Add sample play yards gained to ball position
- Subtract yards gained from distance
- If Distance <= 0:
    - Down = 1
    - Distance = min(10, 100 - ball position)
- Otherwise 
    - Down += 1

### Turnover
- Possession flips to the other team. 
- Because the position is always measure from teh possessing team's own goal line
    - New ball position = 100 - old ball position
- Down = 1
- Distance (for new first down) = min(10, 100 - ball position)

### Turnover on downs
- If 4th down fails to reach the first down marker:
    - Possession flips
    - New ball position = 100 - old ball position
    - Down = 1
    - Distance (for new first down) = min(10, 100 - ball position)

### Touchdown 
- If ball position >= 100:
    - Add 7 points to possessing team (+6 for TD, automatic +1 for PAT)
    - Flip possession
    - Set receiving team's ball position = 25
    - Down = 1
    - Distance = 10

### Field goal attempt 
- Use team field goal rating to simulate a field goal
- On 4th down, if ditance to oppodent goal <= 40:
    - Attempt field goal
- If it was made:
    - Add 3 points to possessiong team
    - Flip possession 
    - Set receiving team's ball position = 25
    - Down = 1
    - Distance = 10
- If missed:
    - Flip ball position from current spot
    - New ball position = 100 - old ball position
    - Down = 1
    - Distance (for new first down) = min(10, 100 - ball position)

### Goal line boundaries

One rule covers both cases: **at or past either goal line is in an end zone.** A touchdown is scored by reaching the plane at 100, and by the same logic a ball at the plane at 0 is in the offense's own end zone.

| Ball position after the play | Normal play | Turnover |
| --- | --- | --- |
| `>= 100` | touchdown | touchback |
| `<= 0` | safety | defensive touchdown |
| `1` to `99` | play on | turnover at the spot |

- **Touchdown**: +7 (6 plus the automatic extra point), kickoff to the opponent.
- **Safety**: +2 to the defense, which then receives the kickoff.
- **Touchback**: possession flips, ball at the receiving team's own 25, 1st and 10, no score. (25 rather than the real-NFL 20, to keep one kickoff spot everywhere. Deliberate simplification, not an oversight.)
- **Defensive touchdown**: the defense recovers in the end zone it is attacking. +7 to the defense, then a kickoff to the original offense. This is the mirror of the safety rule — same spot on the field, different ball outcome: downed with the ball is 2 points, fumbled away is 7.

Consequence worth relying on: a live ball never rests on 0 or 100, so the legal range for `ball_position` between plays is **1 to 99**. Anything outside that is a bug, not an edge case.

Integer yards mean 0 and 100 have to fall on one side of the line or the other; putting them inside the end zone is what makes the table symmetric and matches how touchdowns already worked.

### Safety
- If a play moves the ball position <= 0 (see "Goal line boundaries" above):
    - Add 2 points to the defensive team
    - Possession flipps to that team
    - New offesnse starts on 25 yard line
    - Down = 1
    - Distance = 10

### Opening possession (DECIDED)
- Decision: seeded coin toss at game start, drawn from the same RNG stream used for all play sampling. A 50/50 draw (e.g. `std::bernoulli_distribution(0.5)`) determines who receives first; same seed always reproduces the same result, preserving the fixed-seed reproducibility rule.
- Reason: more realistic than always defaulting to Home, and doesn't cost anything in terms of determinism since it draws from the same seeded stream as everything else.
- GameState.opening_possession field already added (default Home, used only as a placeholder until the coin-toss draw is wired in). Records who received the opening kickoff; the End of quarter/halftime rule depends on this field (see below).

### End of quarter
- If time remaining in quarter <= 0
    - If quarter < 4: 
        - Quarter += 1
        - Time Remaining in quarter = 900
    - If the new quarter is quarter 3:
        - Possession goes to the team that did not start the game with the ball
        - Ball position = 25
        - Down = 1 
        - Distance = 10
    - If quarter 4 ends:
        - Game over = true

### End of game
- After quarter 4 reaches 0:
    - Game over = true
    - Compare home team score with away team score 
    - Winner = (home team, away team, or tie)

## M0.2 Edge cases to test

- Gain exactly the yards for a first down
- Gain more yeards than needed for a first down
- First down inside the oppodents 10 yard line
- Touchdown from exactly ball position 100
- Play moves ball position beyond 100
- Negative yardage play
- Safety caused by negative yardage
- Turnover close to either goal line
- Turnover at midfield
- Failed 4th down at midfield
- Made field goal
- Missed field goal
- Score on final play of a quarter
- Score on the final play of a game
- Q4 ends in a tie
- Same seed reproduces the idential transactions

## Decision log

For later decisions, add a date, decision, reason, and practical consequence. Add detail only when it will help future development.
