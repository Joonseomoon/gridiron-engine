# Gridiron Engine

A C++ NFL simulation engine: play seeded games, repeat matchups to estimate outcome distributions, and later compare
football decisions. See [docs/PROJECT_OVERVIEW.md](docs/PROJECT_OVERVIEW.md) for the goals and architecture.

**Status:** M1 (one seeded game with a play-by-play log) and M2 (`--games N` batches with uncertainty estimates and CSV
export) are complete. M3 (profiling and performance) is in progress.

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
Q1 14:45 - BAL run for 1 yards. 2nd & 9 on BAL 26.
Q1 14:24 - BAL run for 2 yards. 3rd & 7 on BAL 28.
...
Q4 0:00 - Game End.
Final Score
BAL - 10
SEA - 24

Winner: SEA
Seed: 1000
```

The clock shown is after the play. `BAL 28` is the 28-yard line on BAL's half, whichever team has the ball.

**A batch** (`--games N`, 1 to 1,000,000) prints only a summary. `--seed` is then the batch seed, and game `i` (from 0)
uses seed `batchSeed + i`, so game 2 below replays alone as `./build/simulate BAL SEA --seed 1002`.

```
$ ./build/simulate BAL SEA --games 1000 --seed 1000
BAL vs SEA, 1000 games, batch seed 1000
BAL wins: 218   SEA wins: 755   Ties: 27
Mean score: BAL 11.34   SEA 23.39
Mean margin (BAL - SEA): -12.05 Mean total: 34.73
Score sd: BAL 7.76      SEA 10.70       Margin sd: 16.39        Total sd: 9.00
95% CI: BAL win rate 0.218 ± 0.026      Mean margin -12.05 ± 1.02

Convergence:
Games   BAL win rate
10      0.100 ± 0.186
100     0.230 ± 0.082
1000    0.218 ± 0.026
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
0,1000,10,24,away
1,1001,0,51,away
```

Rerunning the recorded command from the same directory gives a byte-identical file with the same build and platform; the
`# model_version:` line says which model produced it, since a newer model gives different games for the same seed. The
parent directory must exist, and the file is written to a temporary file and renamed into place, so a failed run leaves
no partial CSV.

## Limitations

A deliberately simplified game. The rules are decided in [docs/decisions/M0.md](docs/decisions/M0.md); this is what the
simulation does **not** model.

- **Rules:** no punts, penalties, timeouts, two-minute warning, two-point tries, or overtime; games can tie. No kickoffs:
  after every score, at the start of the second half, and on touchbacks, the ball goes to the receiving team's 25.
  4th down: kick a field goal within 40 yards of the goal line, otherwise go for it; a miss turns the ball over at the
  line of scrimmage. Turnovers have no returns. Every touchdown is worth 7.
- **Clock:** running clock only. Every play takes a sampled 10+ seconds, so incomplete passes and out-of-bounds plays
  don't stop it.
- **Model:** play calling is 50/50 run/pass in every situation. Yardage is normal around
  `base + k × (offense rating − defense rating)` with hand-tuned constants (fitted in M5). Turnovers are a flat per-play
  rate. Field goal success falls linearly with distance. Team ratings are hand-set.
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
