# Gridiron Engine

A C++ NFL simulation engine: play seeded games, repeat matchups to estimate outcome distributions, and later compare
football decisions. See [docs/PROJECT_OVERVIEW.md](docs/PROJECT_OVERVIEW.md) for the goals and architecture.

**Status:** M1 (one seeded game with a play-by-play log), M2 (`--games N` batches with uncertainty estimates and CSV
export), and M3 (profiling and multi-threaded performance) are complete. M4 adds replaceable team policies, timeouts,
punts, clock stops, and regular-season overtime; the current model is `m4-v1`.

## Building and testing

Requires CMake 3.20+ and a C++20 compiler (Apple Clang 14+ / GCC 11+). GoogleTest is fetched automatically.

```
cmake -S . -B build                          # configure (re-run if CMakeLists.txt changes)
cmake --build build                          # compile
ctest --test-dir build --output-on-failure   # run all tests
./build/gridiron_tests --gtest_filter='BatchCsvTest.*'   # or run the test binary directly, filtered
```

CI (`.github/workflows/ci.yml`) runs on every push and PR to `main` and fails on any of these. Reproduce them locally
before pushing:

- **Compiler warnings:** `cmake -S . -B build -DGRIDIRON_WERROR=ON`
- **Sanitizer findings:**
  ```
  cmake -S . -B build-asan -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer -fno-sanitize-recover=all"
  cmake --build build-asan && ctest --test-dir build-asan --output-on-failure
  ```
- **Data races:**
  ```
  cmake -S . -B build-tsan -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-fsanitize=thread -fno-omit-frame-pointer"
  cmake --build build-tsan && ctest --test-dir build-tsan --output-on-failure
  ./build-tsan/simulate BAL SEA --games 10000 --seed 1000 --threads "$(sysctl -n hw.ncpu)" > /dev/null
  ```
- **Unformatted code:** use the same pinned clang-format as CI, since versions format slightly differently.
  ```
  pip3 install clang-format==23.1.2
  git ls-files '*.cpp' '*.h' | xargs clang-format -i
  ```

After cloning, run `git config blame.ignoreRevsFile .git-blame-ignore-revs` so `git blame` skips the one-time
reformatting commit.

## Source layout

Includes are written from `src/`, for example `#include "engine/core/GameState.h"`. Tests mirror the source folders.

| Folder | Contents |
| --- | --- |
| `src/engine/core` | Shared types and constants: `GameState`, `Matchup`, `TeamProfile`, play outcomes, `RandomSource`, model constants and version |
| `src/engine/rules` | Applying a play to the game state, timeouts, overtime |
| `src/engine/sampling` | Sampling play outcomes from team ratings |
| `src/engine/policy` | `Policy` and the default, late-game, conservative, and aggressive policies |
| `src/engine/game` | Running or resuming one game, candidate actions, scenario validation |
| `src/engine/batch` | Batches, summaries, per-game results, `parallelFor` |
| `src/cli` | Command-line parsing, built-in teams, game and batch output, CSV export |
| `tests/integration` | Whole-game tests, invariant sweeps, and the M4 sensitivity tables |

## Running

```
simulate HOME AWAY [--seed N] [--games N] [--threads T] [--out FILE]
```

Built-in teams: BAL, SEA, LAR, KC, HOU, DAL (`src/cli/Teams.cpp`), case-insensitive. Invalid input prints an error and
the usage line and exits with code 1.

**One game** prints a play-by-play log and the final score. Without `--seed`, a random seed is used and printed, so any
game can be replayed.

```
$ ./build/simulate BAL SEA --seed 1000
Q1 15:00 - Game Start. BAL receives the football.
Q1 14:32 - BAL run for 3 yards. 2nd & 7 on BAL 38.
Q1 14:01 - BAL run for 6 yards. 3rd & 1 on BAL 44.
Q1 13:26 - BAL run for 0 yards. 4th & 1 on BAL 44.
...
Q4 0:00 - Game End.
Final Score
BAL - 12
SEA - 17

Winner: SEA
Seed: 1000
```

The clock shown is after the play. `BAL 28` is the 28-yard line on BAL's half, whichever team has the ball.

**A batch** (`--games N`, 1 to 1,000,000) prints only a summary. `--seed` is then the batch seed, and game `i` (from 0)
uses seed `batchSeed + i`, so game 2 below replays alone as `./build/simulate BAL SEA --seed 1002`.

```
$ ./build/simulate BAL SEA --games 1000 --seed 1000
BAL vs SEA, 1000 games, batch seed 1000
BAL wins: 314	SEA wins: 674	Ties: 12
Mean score: BAL 20.02	SEA 25.95
Mean margin (BAL - SEA): -5.93	Mean total: 45.97
Score sd: BAL 8.38	SEA 8.97	Margin sd: 13.06	Total sd: 11.44
95% CI: BAL win rate 0.314 ± 0.029	Mean margin -5.93 ± 0.81

Convergence:
Games	BAL win rate
10	0.300 ± 0.284
100	0.320 ± 0.091
1000	0.314 ± 0.029
```

`sd` is how much individual games vary; it doesn't shrink with more games. `95% CI` is the uncertainty of the average
(normal approximation; see [M2 decisions](docs/decisions/M2.md)) and shrinks roughly with √N. With 100 or more games,
the convergence table shows the estimate after the first 10, 100, 1,000, … games. More games only reduce sampling
noise: the interval tightens around what the model predicts, not what would really happen.

**Threads** (`--threads T`, batch only, default 1, at most the hardware thread count) split the batch across T threads.
Output is identical for every T, because game `i` always uses seed `batchSeed + i`. Benchmarks and scaling are in [M3
decisions](docs/decisions/M3.md).

**Export** (`--out FILE`, batch only) also writes every game to a CSV. The file starts with `#` lines recording the
model version, both teams' ratings, the game count, batch seed and thread count, the summary statistics (same values
and precision as the terminal), and a `# command:` line. Then comes the table:

```csv
game,seed,home_score,away_score,winner
0,1000,12,17,away
1,1001,20,24,away
2,1002,31,34,away
```

Rerunning the recorded command from the same directory gives a byte-identical file with the same build and platform; the
`# model_version:` line says which model produced it, since a newer model gives different games for the same seed. The
parent directory must exist, and the file is written to a temporary file and renamed into place, so a failed run leaves
no partial CSV.

## Limitations

A deliberately simplified game. The rules are decided in [docs/decisions/M0.md](docs/decisions/M0.md); this is what the
simulation does **not** model.

- **Rules:** no penalties, two-point tries, playoff overtime, or explicit kickoffs/returns. Touchdowns are worth 7.
  Modeled kickoffs start at the receiving team's 35; punt and turnover touchbacks use the 20. Missed field goals
  turn possession over at the line of scrimmage.
- **Policy:** fourth down attempts a field goal within 40 yards of the goal line; outside range, go for it with
  1–2 yards needed and punt otherwise. Other choices use a 50/50 run/pass draw. A trailing team requests a timeout
  in the last two minutes of Q2, Q4, or overtime when eligible. Each team starts each half with 3 timeouts;
  overtime starts with 2. These are simple, replaceable strategies, not optimized decisions.
- **Clock:** plays take 6 seconds plus sampled runoff. Incompletions, scores, possession changes, timeouts, the
  two-minute warning, and late out-of-bounds plays remove or interrupt runoff. Overtime uses Q4 timing rules.
- **Overtime:** one 10-minute period. Both teams get a possession opportunity subject to the clock; opening
  defensive scores end the game. After the required possessions, the leader wins, or the next score wins if tied.
  Games still tied at expiration remain ties.
- **Model:** normal yardage distributions with hand-set ratings and constants, flat turnover/incompletion rates,
  and a linear field-goal formula. M4 tuning is a rough sanity check, not a fit to NFL data or a validated predictor.
- **Reproducibility:** the same seed and teams give an identical game **on the same platform and standard library**.
  `std::normal_distribution` is implementation-defined, so another compiler or OS can give a different game. Only macOS
  has been tested.

## Documents

- [Project overview](docs/PROJECT_OVERVIEW.md): goals, architecture, and planned versions
- [Roadmap](ROADMAP.md): milestones, rounds, and who builds what
- [Working agreement](CONTRIBUTING.md): workflow, reviews, and issue format
- [Decisions](docs/decisions/README.md): design decisions, one file per milestone
- [Side tasks](docs/TASKS.md): bugs, tooling, and deferred tests
- [Learning record](docs/LEARNING.md): who has implemented what

No open-source license has been selected; agree on one before presenting this as an open-source project.
