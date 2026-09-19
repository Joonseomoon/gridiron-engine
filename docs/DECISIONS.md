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

- Developer A and Developer B names (roles rotate by round):
- Each person's initial learning interests:
- Supported development operating systems:
- C++ standard and compiler baseline:
- Build tool:

- Team input fields:
    - Team Name
    - Offensive Pass Rating
    - Offensive Run Rating
    - Defensive Pass Rating
    - Defensive Run Rating
    - Turnover Rate
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
    - Field goal decision on 4th down is a fixed rule: if the ball is within 40 yards of the opponent's goal line (i.e. distance-to-goal <= 40), attempt a field goal; otherwise, go for it on 4th down. Field Goal Rating is used to sample success; make = +3 and possession flips (kickoff simplification below), miss = turnover on downs at the spot.
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
    - Output: a play-by-play event log (down, distance, ball position, play result) followed by a final line with the final score and winner, e.g.:
      ```
      Final Score
      BAL - 27
      SEA - 21

      Winner: BAL
      Seed: 1000
      ```
    - Acceptance: running the same command twice with the same seed produces an identical log and score; running with a different seed can produce a different result; the game always terminates after 4 quarters.

Prefer play-level simulation if both developers are comfortable with its scope. If choosing drive-level initially, plan to add down-and-distance state before strategy analysis.


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
    - Set receiving team's ball position = 35
    - Down = 1
    - Distance = 10

### Field goal attempt 
- Use team field goal rating to simulate a field goal
- On 4th down, if ditance to oppodent goal <= 40:
    - Attempt field goal
- If it was made:
    - Add 3 points to possessiong team
    - Flip possession 
    - Set receiving team's ball position = 35
    - Down = 1
    - Distance = 10
- If missed:
    - Flip ball position from current spot
    - New ball position = 100 - old ball position
    - Down = 1
    - Distance (for new first down) = min(10, 100 - ball position)

### Safety
- If a play moves the ball position < 0:
    - Add 2 points to the defensive team
    - Possession flipps to that team
    - New offesnse starts on 35 yard line
    - Down = 1
    - Distance = 10

### End of quarter
- If time remaining in quarter <= 0
    - If quarter < 4: 
        - Quarter += 1
        - Time Remaining in quarter = 900
    - If the new quarter is quarter 3:
        - Possession goes to the team that did not start the game with the ball
        - Ball position = 35
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
