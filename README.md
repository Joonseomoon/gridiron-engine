# Gridiron Engine

A C++ NFL simulation and strategy engine with Monte Carlo analysis, performance benchmarks, and historical model evaluation.

## The idea

Build a simplified football simulator, repeat matchups to estimate outcome distributions, and compare decisions within a game. Improve speed and realism in measured iterations. Later versions add historical NFL data, an API and frontend, and optional sports market analytics.

The project gives two developers practical experience with C++, stateful simulation, probability, performance engineering, and model evaluation.

## Current status

Engine skeleton in progress. `applyOutcome` (M1.1) is implemented and tested; the CLI simulator itself is not yet built.

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

Tests also run automatically in CI on every push/PR to `main` (see `.github/workflows/ci.yml`).

## First milestone

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
- [Current round tasks](docs/TASKS.md)
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
