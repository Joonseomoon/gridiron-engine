# Gridiron Engine

A C++ NFL simulation and strategy engine with Monte Carlo analysis, performance benchmarks, and historical model
evaluation.

## The idea

Build a simplified football simulator, repeat matchups to estimate outcome distributions, and compare decisions within a
game. Improve speed and realism in measured iterations. Later versions add historical NFL data, an API and frontend, and
optional sports market analytics.

The project gives two developers practical experience with C++, stateful simulation, probability, performance
engineering, and model evaluation.

## Current status

M1 complete: `simulate` plays a full seeded game from the terminal and prints a play-by-play log and the final result.
M2.1 complete: `simulate --games N` runs a reproducible batch of games and prints win/loss/tie counts and mean scores.
M2.2 in progress: batch CSV export is implemented with recorded run settings and model metadata. Uncertainty estimates
and their integration into the export are still pending.

## Building and testing

Requires CMake 3.20+ and a C++20 compiler (Apple Clang 14+ / GCC 11+). GoogleTest is fetched automatically.

```
cmake -S . -B build          # configure (re-run if CMakeLists.txt changes)
cmake --build build          # compile
ctest --test-dir build --output-on-failure   # run all tests
```

Run the test binary directly for faster iteration or to filter to one test:

```
./build/gridiron_tests
./build/gridiron_tests --gtest_filter=ApplyOutcomeTest.SomeTestName
```

Tests also run automatically in CI on every push/PR to `main` (see `.github/workflows/ci.yml`). CI fails on any of:

- **Compiler warnings.** Locally they are only warnings; reproduce CI with `cmake -S . -B build -DGRIDIRON_WERROR=ON`.
- **Sanitizer findings.** The tests are also run under AddressSanitizer and UndefinedBehaviorSanitizer:
  ```
  cmake -S . -B build-asan -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer -fno-sanitize-recover=all"
  cmake --build build-asan && ctest --test-dir build-asan --output-on-failure
  ```
- **Unformatted code.** Style lives in `.clang-format`. Use the same pinned version as CI, since versions format
  slightly differently:
  ```
  pip3 install clang-format==23.1.2                   # or pipx install clang-format==23.1.2
  git ls-files '*.cpp' '*.h' | xargs clang-format -i  # format everything
  ```
  Most editors (CLion, VS Code) can format on save using `.clang-format`; point them at this version.

After cloning, run `git config blame.ignoreRevsFile .git-blame-ignore-revs` so `git blame` skips the one-time
reformatting commit.

## Running a game

After building:

```
./build/simulate BAL SEA --seed 1000    # BAL at home vs SEA, fixed seed: same game every time
./build/simulate bal sea                # team names are case-insensitive; with no seed, a random one is used and printed
```

Output is a play-by-play log followed by the final result (excerpt):

```
Q1 15:00 - Game Start. BAL receives the football.
Q1 14:34 - BAL run for 1 yards. 2nd & 9 on BAL 26.
Q1 13:41 - BAL pass for 1 yards. Short of the first down. Turnover on downs.
Q1 13:41 - SEA ball on BAL 28, 1st & 10.
...
Q2 14:43 - SEA field goal attempt from 40 yards: GOOD. SEA +3.
...
Q4 0:00 - Game End.
Final Score
BAL - 6
SEA - 26

Winner: SEA
Seed: 1000
```

Each line shows the game clock after the play. Field position names the side of the field: `BAL 28` is the 28-yard line
on BAL's half, whichever team has the ball.

Usage: `simulate HOME AWAY [--seed N] [--games N] [--out FILE]`. Built-in teams: BAL, SEA, LAR, KC, HOU, DAL (defined in
`src/cli/Teams.cpp`). Invalid input prints an error and the usage line, and exits with code 1.

## Running a batch

`--games N` (1 to 1,000,000) plays the same matchup N times and prints only a summary, with no play-by-play:

```
./build/simulate BAL SEA --games 5 --seed 1000
```

```
BAL vs SEA, 5 games, batch seed 1000
BAL wins: 1     SEA wins: 4     Ties: 0
Mean score: BAL 11.20   SEA 24.60
Mean margin (BAL - SEA): -13.40 Mean total: 35.80
```

With `--games`, `--seed` is the batch seed: game `i` (counting from 0) uses seed `batchSeed + i`, so any game in a batch
can be replayed on its own. Game 2 above is `./build/simulate BAL SEA --seed 1002`. Without `--seed`, a random batch
seed is drawn and printed in the first line. The seed policy is decided in [docs/DECISIONS.md](docs/DECISIONS.md).

## Exporting a batch

Add `--out FILE` to save every game's result while still printing the batch summary to the terminal:

```bash
./build/simulate BAL SEA --games 5 --seed 1000 --out results.csv
```

`--out` requires `--games` and may appear only once. Quote paths containing spaces, for example
`--out "my results.csv"`. The destination's parent directory must already exist. The program writes to a temporary
file beside the destination and replaces the destination after writing and closing successfully. File creation,
writing, or rename failures print an error and exit with code 1.

The file begins with `#` comment lines recording the model version, both teams and all six ratings for each team,
game count, actual batch seed, seed policy, and a shell-quoted replay command. The same block records home wins,
away wins, ties, and mean home score, away score, margin, and total. Means are written to two decimal places.

After those comments, the CSV table has this header and one row per game (excerpt from the example above):

```csv
game,seed,home_score,away_score,winner
0,1000,6,26,away
1,1001,3,32,away
2,1002,18,16,home
```

`game` is a zero-based index; `winner` is `home`, `away`, or `tie`. Each row records the seed actually used for that
game. CSV readers should skip the initial lines beginning with `#` before reading the column header.

To reproduce a batch, copy the command after `# command:` and run it from the same working directory. If the original
run omitted `--seed`, the recorded command includes the generated seed. With the same executable/model, team ratings,
platform, and standard library, replay produces a byte-identical file. For example, save a copy before replaying:

```bash
cp results.csv first.csv
# Run the command recorded in results.csv, then compare:
cmp first.csv results.csv
```

`cmp` prints nothing and exits successfully when the files are identical. The recorded model version identifies the
model used; it does not load an older model or restore ratings. Configuration-file loading is not supported.

## Limitations

M1 is a deliberately simplified game. The rules below are decided in [docs/DECISIONS.md](docs/DECISIONS.md); this is the
reader's summary of what the simulation does **not** model.

**Football rules**

- No punts, penalties, timeouts, two-minute warning, two-point tries, or overtime. Games can end in a tie.
- Kickoffs are not played: after every score, and to start the second half, the receiving team starts at its own 25.
  Touchbacks also go to the 25.
- 4th down is a fixed rule: within 40 yards of the goal line, kick a field goal; otherwise go for it. A missed field
  goal turns the ball over at the line of scrimmage.
- Turnovers have no returns. The ball changes hands where the play ended; a defensive touchdown only happens when that
  spot is in the offense's own end zone.
- Every touchdown is worth 7 (the extra point is automatic).

**Clock**

- Running clock only. Each play takes a sampled 10+ seconds whatever happened, so incomplete passes and out-of-bounds
  plays don't stop it.

**Probability model**

- Play calling is a coin flip: 50% run, 50% pass, regardless of down, distance, score, or time.
- Yardage is drawn from a normal distribution around `base + k × (offense rating − defense rating)`. The constants are
  hand-tuned guesses, not fitted to data (that is M5).
- Turnovers are a flat per-play rate from the offense's rating, the same for runs and passes and in every situation.
- Field goal success falls linearly with distance from a hand-set rating.
- Team ratings are hand-set for six built-in teams (`src/cli/Teams.cpp`).

**Reproducibility**

- Same seed and teams give an identical game **on the same platform and standard library**. The yardage and clock draws
  use `std::normal_distribution`, whose output is implementation-defined, so a seed can produce a different game with
  another compiler or OS. Only macOS has been tested.

## First milestone (complete)

Run one complete simplified game from the terminal and print a final score and readable game log. Use hand-set team
ratings and a fixed random seed. Agree on rule simplifications before implementing them.

## Why C++

The simulation loop offers opportunities to study memory layout, allocation, random-number generation, and CPU
parallelism. We will measure performance rather than assume it. Data preparation may use Python; the reusable simulation
engine stays in C++.

## Working together

Both developers build substantive functionality at every milestone. Each round has two parallel task groups, agreed
shared inputs and outputs, and an integration checkpoint. Lead areas rotate within milestones so both people implement
systems, statistical, data, and interface work. We integrate a small working path early and expand it together. Tests
belong to both authors.

## Roadmap

0. Agree on scope and shared inputs and outputs.
1. Simulate one game.
2. Repeat matchups and summarize results.
3. Profile and improve performance.
4. Compare football decisions.
5. Fit and evaluate models with real NFL data.
6. Expose the engine through an API and frontend.
7. Evaluate sports markets historically.
8. Package a reproducible release.

See [ROADMAP.md](ROADMAP.md) for responsibilities, integration points, and completion criteria.

## Project documents

- [Working agreement](CONTRIBUTING.md)
- [Project overview](docs/PROJECT_OVERVIEW.md)
- [Initial decisions](docs/DECISIONS.md)
- [Side tasks: bugs, tooling, deferred tests](docs/TASKS.md)
- [Learning and rotation record](docs/LEARNING.md)

## Principles

- Keep main working and merge small changes frequently.
- Establish correctness before optimization.
- Separate football rules from probability models.
- Record assumptions, seeds, model versions, and evaluation settings.
- More simulations reduce sampling noise; they do not fix a poor model.
- Treat market analytics as research, with no promise of betting profit.

## License

No open-source license has been selected. Agree on one before presenting this as an open-source project.
