# Side tasks

Work that is not in the ROADMAP.md tables: bugs found along the way, tooling, and deliberately deferred tests. Roadmap
work is tracked in ROADMAP.md and its issues, not here. Remove an item once it is merged.

## Bugs

## Deferred

### M4 checkpoint tool and data (from PR #50)

`scripts/m4_checkpoint.cpp` is built by hand, outside CMake, so CI never compiles it and it can break silently. Either
add it as a CMake target (and a CI build step) or delete it once 4.3's comparison runner can report the same
statistics. `docs/validation/m4-checkpoint.json` is 716 lines of raw checkpoint output; the numbers that matter are in
`decisions/M4.md`, so delete it unless a later milestone needs the raw data.

### Late-game policy follow-ups (from PR #55)

- **Behavior:** a trailing team in the last 2:00 of Q4 still punts when outside field-goal range (for example down 7,
  0:40 left, 4th and 8 at its own 30), because only the within-40-yards case is handled. Go for it there instead, with a
  test and a line in `decisions/M4.md`. This changes the checkpoint win values, so rerun and update them.
- Move the literal 60, 120, and 40 in `LateGamePolicy.cpp` into `ModelConstants.h` (`kTimeoutWindowSeconds` already
  covers 120).
- Tests: the M4.md example state, a defense that is not trailing, and the defense's 0-second timeout edge. Fix the
  "Offesnse" typo in two test names.
- Add B's M4.2 entry to `LEARNING.md`.

### Small code cleanups (from PRs #48–#49)

- GameLog prints overtime as "Q5" and hard-codes "Each team has 2 timeouts" instead of `kOvertimeTimeouts`.

### Source layout (start of M5)

`src/engine` and `tests` are flat (31 and 17 files after 4.2). At the start of M5, when data and model code need new
folders anyway, sort them into subfolders (for example rules, sampling, policy, run) with tests mirroring them. Do it as
a move-only PR between rounds, with no other branches open, using `git mv` so history is kept. Add a CMake include
directory so includes become `"engine/..."` instead of `"../src/engine/..."`. Fingerprints must not change.

### clang-tidy

Static analysis beyond compiler warnings and sanitizers; it could also mechanically enforce one naming convention (
locals currently mix camelCase and snake_case). Noisy to set up, so it waits until M3, when performance work and
threading make it pay off. Start with a narrow check set (`bugprone-*`, `performance-*`) rather than everything.
