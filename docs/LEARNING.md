# Learning and Rotation Record

Update this briefly at each milestone boundary. Record actual implementation work and a pull request or concrete example. Reviewing or watching helps understanding but is not a substitute for building something.

| Area | A implemented | B implemented | Useful next crossover |
| --- | --- | --- | --- |
| Game state and football rules | M1.1: `applyOutcome` — down/distance, turnovers, scoring, goal-line and safety detection, quarter/halftime/game-over transitions (PR #7, #9); M1.3: second half always starts with the non-opening team, even after a score on the last play of the half (PR #19) | M1.2: `runGame` game loop with play-count bound, seeded opening coin toss, termination and reproducibility tests (PR #14); M1.3: `PlayResult` — `applyOutcome` now reports what happened (scoring event, turnover kind, quarter/halftime/game end) (PR #21) | Crossover done both ways in this area |
| Probability sampling and team models | M1.2: 4th-down field-goal decision and distance-dependent make/miss sampling; extended the sampler test suite after mutation testing exposed it as mostly vacuous | M1.1: `OutcomeSampler` — run/pass yardage from team ratings, per-play turnover sampling, seeded RNG, play duration (PR #9) | Crossover done both ways in this area. B could take the golden/draw-order test that is still deferred |
| Batch execution and reproducibility | | | |
| Statistics and uncertainty | | | |
| Profiling and engine optimization | | | |
| Parallel execution | | | |
| Strategy policies and analysis | | | |
| Data preparation and quality | | | |
| Model fitting and feature evaluation | | | |
| Calibration and historical evaluation | | | |
| Model loading and runtime integration | | | |
| API and service behavior | | | |
| Frontend and result presentation | M1.3: `simulate` command line — argument parsing with strict seed validation, case-insensitive team selection from a built-in table, generated-and-printed seed, error messages and exit codes, with tests (PR #19) | M1.3: play-by-play log and final result block (`GameLog`), wired into `simulate` (PR #21); tests carried over into M2.1 | Both have built user-facing output; M6 frontend work can go to either |
| Market calculations and backtesting | | | |
| Build, tests, documentation, and releases | M0/M1: GoogleTest via CMake FetchContent, GitHub Actions CI, README build/test instructions, M1.1 integration test (PR #6); M1.3: split the build into `gridiron_core` (engine) and `gridiron_cli` libraries, documented CLI decisions in DECISIONS.md and README (PR #19) | M1.1: CMake target wiring for the sampler and its tests (PR #9) | B owns the next build/docs task so this area is not A-only |

## How to use this

- Choose next-round assignments based on interests and gaps, while preserving dependencies.
- The roadmap supplies default crossovers; change them by mutual agreement.
- Prefer meaningful extensions over rewriting completed work for practice.
- Both own tests and documentation throughout the project.
- Breadth does not require identical time, every subcomponent, or equal expertise.
- If one area still has only one implementer, schedule a useful small extension there for the other person.
