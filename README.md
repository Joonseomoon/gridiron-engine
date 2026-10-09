# Gridiron Engine

A C++ NFL simulation and strategy engine with Monte Carlo analysis, performance benchmarks, and historical model evaluation.

## The idea

Build a simplified football simulator, repeat matchups to estimate outcome distributions, and compare decisions within a game. Improve speed and realism in measured iterations. Later versions add historical NFL data, an API and frontend, and optional sports market analytics.

The project gives two developers practical experience with C++, stateful simulation, probability, performance engineering, and model evaluation.

## Current status

M1 complete: `simulate` plays a full seeded game from the terminal and prints a play-by-play log and the final result. Next is M2, running repeated matchups and summarizing the results. A few M1 follow-ups are tracked in [docs/TASKS.md](docs/TASKS.md).

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
- **Unformatted code.** Style lives in `.clang-format`. Use the same pinned version as CI, since versions format slightly differently:
  ```
  pip3 install clang-format==23.1.2                   # or pipx install clang-format==23.1.2
  git ls-files '*.cpp' '*.h' | xargs clang-format -i  # format everything
  ```
  Most editors (CLion, VS Code) can format on save using `.clang-format`; point them at this version.

After cloning, run `git config blame.ignoreRevsFile .git-blame-ignore-revs` so `git blame` skips the one-time reformatting commit.

## Running a game

After building:
```
./build/simulate BAL SEA --seed 1000    # BAL at home vs SEA, fixed seed: same game every time
./build/simulate bal sea                # team names are case-insensitive; with no seed, a random one is used and printed
```
Output is a play-by-play log followed by the final result (excerpt):
```
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
Each line shows the game clock after the play. Field position names the side of the field: `BAL 28` is the 28-yard line on BAL's half, whichever team has the ball.

Usage: `simulate HOME AWAY [--seed N]`. Built-in teams: BAL, SEA, LAR, KC, HOU, DAL (defined in `src/cli/Teams.cpp`). Invalid input prints an error and the usage line, and exits with code 1.

## First milestone (complete)

Run one complete simplified game from the terminal and print a final score and readable game log. Use hand-set team ratings and a fixed random seed. Agree on rule simplifications before implementing them.

## Why C++

The simulation loop offers opportunities to study memory layout, allocation, random-number generation, and CPU parallelism. We will measure performance rather than assume it. Data preparation may use Python; the reusable simulation engine stays in C++.

## Working together

Both developers build substantive functionality at every milestone. Each round has two parallel task groups, agreed shared inputs and outputs, and an integration checkpoint. Lead areas rotate within milestones so both people implement systems, statistical, data, and interface work. We integrate a small working path early and expand it together. Tests belong to both authors.

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
