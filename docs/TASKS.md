# Side tasks

Work that is not in the ROADMAP.md tables: bugs found along the way, tooling, and deliberately deferred tests. Roadmap
work is tracked in ROADMAP.md and its issues, not here. Remove an item once it is merged.

## Carry-over from M1.3 (finish during M2.1)

The M1.3 checkpoint passed acceptance criteria 1–5 and 8 from DECISIONS.md. These items close the rest.

### Acceptance criteria 6 and 7 (Developer A)

- **#6 Winner correctness.** The winner is computed inline in `main.cpp`, where no test can reach it. Move it into a function (e.g. build a `SimulationResult` from the final `GameState` and the seed) and test a home win, an away win, and a tie.
- **#7 Rule fidelity.** The log never says who received the opening kickoff, so the coin toss is invisible. Print an opening line before the first play, as in the DECISIONS.md sample: `Q1 15:00 - BAL ball on BAL 25, 1st & 10`.

### Tests for the play log (Developer B)

- **`PlayResult` tags.** Only 3 `applyOutcome` tests check `event` / `turnover` / `period`. Add a tag assertion for each case: touchdown, field goal good and missed, safety, touchback, interception vs fumble, defensive touchdown, turnover on downs, end of quarter, halftime, end of game. Optionally, check tag consistency in the invariant sweep (e.g. a touchdown tag always means the offense gained 7).
- **`printPlay` / `printFinalResult`.** No tests yet. Print into a `std::ostringstream` and compare the text: one line per event type, field position on both sides and at midfield, `& goal`, the clock at `0:00`, and a tie in the final block.

### Write down M1's limitations (at the M2.1 checkpoint)

CONTRIBUTING.md asks to record limitations before moving on. Collect them in one place (a "Limitations" section in README.md is the natural home) so a reader of the demo knows what it does not model: no punts, penalties, timeouts, two-point tries, or overtime; kickoffs are a fixed spot at the 25; a running clock; a 50/50 run/pass split; flat per-play turnover rate; hand-set team ratings and yardage constants (to be fitted in M5). Most are already decided in DECISIONS.md; this is a summary for readers, not new decisions. Note criteria 6 and 7 here until the carry-over above lands.

## Bugs

## Deferred

### clang-tidy

Static analysis beyond compiler warnings and sanitizers; it could also mechanically enforce one naming convention (
locals currently mix camelCase and snake_case). Noisy to set up, so it waits until M3, when performance work and
threading make it pay off. Start with a narrow check set (`bugprone-*`, `performance-*`) rather than everything.
