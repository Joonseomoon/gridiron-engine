# Project Overview

## Purpose

Simulate NFL games and estimate distributions of possible outcomes. Use the same engine to compare football decisions and eventually study historical market prices. The initial goal is a reproducible learning project for two developers, with measurable improvements in correctness, speed, and modeling.

## High-level architecture

Team inputs and model parameters → game state and rules → sampled outcomes and decision policies → simulation results → batch analysis → terminal interface or API and frontend.

Historical data preparation and model fitting produce versioned parameters. Evaluation compares saved predictions with later outcomes. Keep data processing separate from the simulation loop.

## Technical interest

Football requires state transitions involving possession, downs, distance, field position, score, and time. Repeated simulations add probability estimation and reproducibility. Performance work adds profiling and parallel execution. Strategy analysis introduces alternative actions and continuation policies. Real data makes calibration and honest historical evaluation essential.

## Eventual market extension

For decimal odds d, implied break-even probability is 1/d for an outcome without a push. American odds +A imply 100/(A+100); odds -A, with A as the positive magnitude, imply A/(A+100). A simple margin adjustment normalizes quoted probabilities across a complete set of mutually exclusive outcomes by their sum.

An illustrative model estimate of 58% against decimal odds of 2.00 differs from the quoted 50% break-even probability by 8 percentage points. This is a model-dependent comparison, not proof of profit. Model-implied fair decimal odds would be approximately 1.72.

Use score margins for spread coverage and combined scores for totals. Handle pushes, ties, and overtime consistently with each market's settlement definition.

Backtests must use only information and odds available at prediction time. Train and evaluate chronologically, preserve an untouched final test period, and predefine any hypothetical staking rule. Report calibration, probability metrics, sample sizes, uncertainty, and unfavorable results as well as favorable ones.

A calibrated model's predictions around 60% should occur around 60% of the time across enough comparable cases. Millions of simulations can still produce inaccurate probabilities when the underlying model is wrong.

Begin with historical research and paper tracking. Automated wagering is outside scope. Do not promise profits; any later real-money use needs separate consideration of applicable rules, data permissions, and personal loss limits.
