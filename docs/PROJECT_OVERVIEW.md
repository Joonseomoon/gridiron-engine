# Project Overview

What we are building and why. Task assignments and rounds are in [ROADMAP.md](../ROADMAP.md); how we work together is
in [CONTRIBUTING.md](../CONTRIBUTING.md); specific design decisions are in [DECISIONS.md](DECISIONS.md).

## Purpose

Build a C++ engine that simulates NFL games and repeats matchups thousands or millions of times to estimate outcomes and
compare football decisions. Start with a small, understandable model, then improve speed, realism, and usability in
separate iterations.

The eventual product accepts teams or a game situation and returns a distribution of possible scores, win
probabilities, and strategy comparisons. A later extension compares model estimates with historical betting markets.
It is a reproducible learning project for two developers, with measurable progress in correctness, speed, and modeling.

## Why it is technically interesting

Football is a stateful simulation problem: possession, field position, down, distance, score, and remaining time all
influence what happens next. Repeated simulations add probability estimation, uncertainty, reproducibility, and parallel
execution. Strategy analysis adds decision policies; real data adds model fitting and validation. Success means showing
both software correctness and useful statistical behavior.

## Why C++

C++ gives control over memory layout, allocation, random-number generation, and CPU parallelism in the simulation loop,
which makes profiling and optimization a substantial part of the project. Speed still depends on implementation and
measurement. Python can support data preparation and model experiments, and a web frontend can present results; the
simulator itself stays a reusable C++ library.

## Architecture

Historical data → preparation and model fitting → versioned model parameters → C++ simulation engine → batch runner and
analysis → command line, then API and frontend.

- **Game state and rules:** the clock, possession, downs, field position, score, and game termination. Season-specific
  rules stay configurable as realism grows.
- **Outcome model and policies:** sample play results and choose actions through replaceable interfaces, so assumptions
  can change without rewriting the engine.
- **Simulation runner:** initialize scenarios, assign reproducible random streams, run batches, and aggregate score
  distributions and uncertainty estimates.
- **Evaluation and delivery:** validate historical predictions, save experiment metadata, and expose stable inputs and
  results to the command line and later the API.

Data processing stays separate from the simulation loop.

## Versions

Each version matches the roadmap milestone with the same number. Example outputs are illustrative, not measured
predictions or performance promises.

1. **Basic game simulator.** One complete game from simple team ratings and explicit probabilities for passing,
   rushing, turnovers, and scoring, with downs, possession, field position, and time. Documented simplifications for
   penalties, special teams, and overtime. Fixed seed and a readable game log. *Done when* `simulate BAL SEA` gives a
   final score and traceable events, with tests for scoring, possession changes, downs, and game completion.
2. **Monte Carlo matchup simulator.** Run a matchup many times and aggregate wins, losses, ties, margins, and totals,
   with sample size and Monte Carlo uncertainty. More runs reduce sampling noise, not errors in football assumptions.
   *Done when* a batch gives repeatable summary statistics from recorded inputs, model version, and seeds.
3. **Performance engineering.** A single-thread baseline, profiling, data layout, fewer allocations, random-number
   generation, and a thread pool with independent random streams and local aggregation. Seeds assigned by simulation
   index keep results reproducible across worker counts. *Done when* simulations per second, elapsed time, and scaling
   across thread counts are published for named hardware, and optimizations preserve correctness and distributions.
4. **Strategy analysis.** Resume from a supplied game state and compare going for it, punting, or a field goal, by
   simulating the rest of the game under an explicit continuation policy. Adds timeouts and clock behavior where needed.
   *Done when* a situation such as trailing 27–24 with 3:14 left, fourth-and-2 at the opponent's 43, can be analyzed,
   with an explanation of how the recommendation depends on model assumptions.
5. **Real NFL data.** Replace hand-set probabilities with estimates from historical play-by-play data, using context
   such as down, distance, field position, and team strength; smooth sparse situations. Data preparation stays outside
   the C++ runtime. *Done when* chronological train/evaluation splits, a simple baseline comparison, and checks of score
   distributions and probability calibration are in place.
6. **API and frontend.** Matchup simulation and strategy analysis through an API backed by the same library, plus a
   small interface for teams, game states, and score distributions. Input validation, bounded requests, and background
   jobs for long runs. *Done when* both developers can demonstrate a matchup and a strategy comparison end to end, with
   visible assumptions, model version, run count, and uncertainty.
7. **Market analytics.** An optional research layer comparing estimated probabilities with historical market prices
   (below). It follows a validated simulator.

## Market extension

The purpose is to study whether the model contains useful information and whether apparent advantages survive honest
testing, preserving uncertainty in every comparison.

**Implied probability.** For decimal odds d, the break-even probability is 1 / d. For American odds +A it is
100 / (A + 100); for −A, with A the positive magnitude, it is A / (A + 100). Quoted probabilities include bookmaker
margin; for a complete set of mutually exclusive outcomes, dividing each by their sum gives a simple margin-adjusted
estimate. Example: a model estimate of 58% against decimal odds of 2.00 (50%) is an 8-point discrepancy, with
model-implied fair odds of about 1.72. That is a model-dependent comparison, not proof of an exploitable edge. Keep both
sides' prices and timestamps, and compare identical outcome and settlement definitions.

**Spreads and totals.** Simulated margins give spread coverage and combined scores give over/under probabilities. BUF
−3.5 covers when Buffalo wins by at least four; over 48.5 needs at least 49 combined points. Handle pushes on
whole-number lines separately, and match overtime and tie treatment to the market.

**Backtesting.** Use only information and odds actually available at each prediction time; closing prices cannot stand
in for earlier quotes. Train on earlier periods, evaluate on later ones, and keep a final untouched test period. Report
Brier score or log loss, calibration, results by season, sample sizes, data gaps, and bookmaker margin. For hypothetical
betting results, predefine a simple staking rule and report return, drawdowns, and uncertainty against simple forecasts
and margin-adjusted market probabilities. Don't select strategies after seeing test outcomes.

**Calibration.** Outcomes labeled around 60% should happen about 60% of the time across enough comparable cases; check
with reliability plots on held-out data. Monte Carlo sampling error is separate from model error: millions of
simulations can still give confidently wrong probabilities.

**Framing.** Analytics and education, starting with historical research and paper tracking. No promise of profits and
no automated wagering. Any later real-money use needs separate consideration of applicable rules, data permissions, and
personal loss limits.
