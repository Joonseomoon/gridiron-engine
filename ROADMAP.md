# Sequential Roadmap with Rotating Roles

## How to read this plan

Milestones run in order. Each milestone contains smaller rounds. Within a round, A and B work in parallel; at the end, they integrate and review before starting work that depends on that round. A and B identify people for planning purposes, not permanent specialties. Either person may take either column, provided the next round preserves the planned change in experience.

Both people should implement meaningful work across engine logic, probability and statistics, data preparation, modeling, performance, services, frontend, tests, and documentation. Reviewing the other person's work helps, but does not replace hands-on implementation. Neither person must master every detail or spend exactly equal time in each area.

### The round workflow

1. Together, agree on the shared inputs and outputs, one example, and the completion check.
2. Make any small shared prerequisite available first: a build skeleton, data schema, sample result, or stable interface.
3. Each person takes one task that can start using that agreed baseline. Every issue names its prerequisites and a fallback sample or existing implementation.
4. Build in parallel, with each author owning relevant tests and documentation. Avoid assigning both tasks changes to the same central file.
5. Connect a minimal path early. Merge small reviewed changes and demonstrate the combined result.
6. Switch areas in the next round. The previous lead gives a short walkthrough; the new lead owns a real extension or improvement.

The tables are task groups, not instructions to create a single large pull request. Split them further when necessary. If one group is much larger, divide or rebalance it before starting. Most implementation time should be parallel; initial decisions, necessary shared setup, integration, and final checks are intentionally joint or sequential. Do not pretend that fitting can precede data availability or that evaluation can precede predictions.

## M0 Agree on the first game

**Goal:** A limited, concrete definition of V1.

| Round | Developer A | Developer B | Shared checkpoint |
| --- | --- | --- | --- |
| 0.1 Define the example | Propose game state, progression, and simplified football rules. | Propose team inputs and terminal results for the same example game. | Walk from input to final score and agree on the common definitions. |
| 0.2 Cross over | Turn the agreed example into user-facing acceptance criteria and a sample terminal interaction. | Turn the agreed rules into a list of state-transition cases and edge cases. | Complete DECISIONS.md, choose the build approach, and prepare M1 issues. |

**Parallel starting point:** Both use the same written example; no implementation depends on the other person's draft.

**Done:** Both can describe the first demo and the intentionally simplified rules.

## M1 Build one complete game

**Shared prerequisite:** Pair on the minimal build skeleton and agree on game state, play outcome, team input, and result formats. Use a play-level V1 unless the agreed scope explicitly chooses otherwise.

| Round | Developer A | Developer B | Shared checkpoint |
| --- | --- | --- | --- |
| 1.1 One event (done) | Build state transitions for applying an outcome, using fixed example outcomes. | Build the initial pass/run outcome sampler and team ratings, using fixed example states. | Sample one event, apply it, and inspect the new state. |
| 1.2 Complete the game and switch areas | Extend the probability model: 4th-down field goal decision and make/miss sampling from the field goal rating, plus checks that sampled turnover and field goal rates hold in aggregate. | Extend game progression: the game loop driving plays to termination, the seeded opening coin toss, and a termination test. | Run complete games using both extensions; sweep many seeds and assert every intermediate state is legal (down 1-4, distance >= 0, ball position 0-100, no 5th quarter). |
| 1.3 Make it usable | Build terminal input handling, team selection, and seed options. | Build the readable event log and final result presentation. | Connect both to the already working game and run from a fresh checkout. |

**Parallel starting point:** Round 1.1 uses agreed state/outcome examples. Round 1.2 uses the merged one-event implementation and an outcome vocabulary agreed before splitting; its own shared prerequisite is giving the sampler access to game state and agreeing who maps possession to team profiles, since both halves depend on that interface. Round 1.3 uses the completed engine and fixed result format.

The turnover-past-the-goal-line fix (see docs/KNOWN_ISSUES.md) is unassigned and must land in 1.2, since the checkpoint's legality sweep covers exactly that bug. Task-level detail for the current round lives in [docs/TASKS.md](docs/TASKS.md).

**Done:** Both run a complete seeded game, have implemented rules and probability behavior, and can explain simplifications. Each adds meaningful tests for their changes.

## M2 Run repeated matchups

**Shared prerequisite:** Agree on the single-game result and batch summary formats, seed policy, and saved run metadata.

| Round | Developer A | Developer B | Shared checkpoint |
| --- | --- | --- | --- |
| 2.1 Produce and summarize | Build batch orchestration, clean initialization, and repeatable seed assignment. | Build aggregation of win/loss/tie counts, scores, margins, and totals from example game results. | Send a handful of actual games through the summary path. |
| 2.2 Switch areas | Add Monte Carlo uncertainty estimates and convergence summaries to the merged aggregation code. | Add configurable run settings, result export, and recorded model/seed metadata to the merged runner. | Run one reproducible batch with statistics and saved configuration. |

**Parallel starting point:** Round 2.1 uses the agreed game-result format. Both tasks in 2.2 extend separate parts of the working batch path.

**Done:** Counts reconcile with completed games. Both have built execution/reporting infrastructure and statistical functionality across M1–M2. Explain that more samples reduce sampling noise, not model bias.

## M3 Measure and improve performance

**Shared prerequisite:** Establish a repeatable single-thread benchmark using the M2 runner. Record workload and hardware, inspect a profile together, and agree on reproducibility conditions.

| Round | Developer A | Developer B | Shared checkpoint |
| --- | --- | --- | --- |
| 3.1 Improve separate bottlenecks | Optimize one measured engine hot path without changing its public behavior. | Add parallel batch execution with independent random streams and safe aggregation, using the unchanged engine interface. | Benchmark each change independently and then together; check correctness and distributions. |
| 3.2 Switch areas | Extend the parallel runner with measured scheduling or batch-size improvements if profiling supports them. | Optimize a different measured engine bottleneck, such as allocation or data access, if evidence supports it. | Compare against the same baseline and document what helped or did not help. |

**Parallel starting point:** A stable engine call lets runner work and internal engine work proceed independently. In 3.2 choose distinct files/components before starting. If no further optimization is justified, use that task to investigate and document the new area's limits instead of forcing a change.

**Done:** Both have worked on engine profiling and parallel execution. Publish reproducible measurements; do not require an arbitrary speed target.

## M4 Compare football decisions

**Shared prerequisite:** Agree on scenario state, candidate actions, continuation-policy inputs/outputs, and comparison results. Add any missing foundational down/distance state together before the parallel work.

| Round | Developer A | Developer B | Shared checkpoint |
| --- | --- | --- | --- |
| 4.1 Resume and choose | Build scenario initialization and candidate-action execution using a simple existing/default continuation policy. | Build an explicit policy for subsequent decisions using example states and the agreed policy interface. | Resume one scenario under two actions and finish the game. |
| 4.2 Switch areas | Extend the policy analysis with a controlled sensitivity comparison, such as conservative versus aggressive later decisions. | Build the scenario comparison runner, input checks, win-probability differences, and uncertainty output using the merged default policy. | Add the new sensitivity cases to the working comparison report. |

**Parallel starting point:** Each uses agreed interfaces and a default policy; neither requires the other's new policy variant to start.

**Done:** Both have worked on simulation mechanics and decision analysis. Explain how recommendations depend on continuation policies and uncertain inputs.

## M5 Model with real NFL data

**Shared prerequisite:** Confirm data provenance and permitted use. Inspect and freeze a small sample; agree on schema, prediction time, target outcome, chronological splits, and model artifact format. This initial data access is a real dependency, so resolve it before parallel modeling.

| Round | Developer A | Developer B | Shared checkpoint |
| --- | --- | --- | --- |
| 5.1 First fitted outcome | Fit a simple model for one selected outcome from the frozen sample; export parameters in the agreed format. | Build repeatable preparation and quality checks that reproduce the sample schema from the available source data. | Fit using the prepared output and verify the same field meanings throughout. |
| 5.2 Switch model and data roles | Extend preparation with one agreed context feature and missing-data handling, preserving the baseline schema. | Fit and validate a second simple model or meaningful baseline-model improvement using existing fields first. | Evaluate the added feature after integration on validation data; preserve a baseline comparison. |
| 5.3 Runtime and evaluation | Build engine parameter loading/version checks against a frozen example model artifact. | Build historical evaluation, calibration summaries, and probability metrics against agreed prediction records. | Run one fitted model through the engine and historical evaluator. |
| 5.4 Switch runtime and evaluation roles | Extend evaluation with season/context breakdowns and score-distribution checks. | Extend engine model integration with fallback behavior for sparse or missing contexts and recorded model metadata. | Re-run the combined path and inspect failure cases together. |

**Parallel starting point:** 5.1 starts from a shared sample. In 5.2 the second model must work with existing fields; the new feature is an optional later experiment. 5.3 uses frozen model and prediction examples. 5.4 extends the merged baseline in separate components.

**Done:** Both have implemented data preparation, model work, and runtime/evaluation functionality. Use chronological validation, prevent future-data leakage, and keep a final test period untouched by model selection. Report whether the baseline improved, including negative findings.

## M6 Add an API and frontend

**Shared prerequisite:** Agree on request/response examples, errors, and the current engine capabilities. Set up the minimal service and frontend shells together.

| Round | Developer A | Developer B | Shared checkpoint |
| --- | --- | --- | --- |
| 6.1 Matchup | Build the matchup API and engine connection. | Build matchup inputs and result displays using the agreed response example. | Connect one real request early, then finish the matchup feature. |
| 6.2 Strategy and switch sides | Build scenario entry and strategy comparison views. | Build the strategy API using the merged service pattern. | Connect one real strategy request early and finish the comparison feature. |
| 6.3 Long runs | Build service-side job status, limits, and error behavior if long runs require it. | Build progress, completion, and error displays against the agreed job-state examples. | Demonstrate a bounded long-running request end to end. |

**Parallel starting point:** Each round begins with a fixed contract and example responses. Mocks unblock the first UI work only; connect actual responses within the round.

**Done:** Both have implemented backend and frontend functionality. The product shows sample size, model version, assumptions, and uncertainty. Avoid adding job infrastructure if observed runtimes do not justify it.

## M7 Evaluate sports markets historically

**Shared prerequisite:** Confirm access to timestamped odds. Freeze a few example games and agree on market schema, decision time, settlement rules, prediction format, and backtest policy before evaluating results.

| Round | Developer A | Developer B | Shared checkpoint |
| --- | --- | --- | --- |
| 7.1 Price comparison | Build implied probability conversion, documented margin adjustment, and model-to-price comparisons using sample quotes. | Build odds preparation, timestamp validation, and prediction-price matching using the sample schema. | Trace a few games from available information to quoted price and comparison. |
| 7.2 Switch areas | Build chronological backtest orchestration and saved experiment settings using the merged market records. | Extend probability and settlement calculations to spreads, totals, pushes, and relevant tie/overtime treatment. | Run the new markets through the backtest; retain the simpler market as a baseline. |
| 7.3 Evidence and reproducibility | Build calibration, sample-size, uncertainty, and hypothetical return/drawdown summaries from fixed backtest records. | Build reproducible experiment execution and result export using the existing runner and agreed result format. | Produce a complete saved historical evaluation and inspect unfavorable results too. |

**Parallel starting point:** 7.1 uses frozen quotes. In 7.2 the runner starts with the already supported market while new markets are added independently. 7.3 uses a stable backtest result schema.

**Done:** Both have built market calculations and data/experiment infrastructure. Predefine any hypothetical staking rule, prevent leakage, and distinguish model accuracy from simulated profitability. Begin with research and paper tracking; automated wagering remains outside scope.

## M8 Package a reproducible release

**Shared prerequisite:** Review the learning record. Identify any area one person has only reviewed, and choose a useful small implementation task there before release. Do not invent features solely to fill a checkbox.

| Round | Developer A | Developer B | Shared checkpoint |
| --- | --- | --- | --- |
| 8.1 Complete and explain | Take a bounded improvement in a less-practiced area and document its demo. | Take a separate bounded improvement in a less-practiced area and document its demo. | Review both changes and verify their connection to the existing product. |
| 8.2 Reproduce | Follow B's setup/demo instructions from a fresh checkout and fix gaps in the corresponding path. | Follow A's setup/demo instructions from a fresh checkout and fix gaps in the corresponding path. | Finish release notes, limitations, data requirements, and the license decision together. |

**Parallel starting point:** Choose independent improvements and cross-check separate demo paths. Coordinate edits to shared setup documents.

**Done:** Someone else can run the documented demos. Both have meaningful hands-on exposure across the project, and claims are supported by saved evidence. Tag a release and select future work from observed limitations.
