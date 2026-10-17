# Side tasks

Work that is not in the ROADMAP.md tables: bugs found along the way, tooling, and deliberately deferred tests. Roadmap
work is tracked in ROADMAP.md and its issues, not here. Remove an item once it is merged.

## Bugs

### Exported CSV is readable only by its owner

`--out` writes to a temporary file from `mkstemp`, which creates it with mode `0600`, then renames it over the
destination. So every exported CSV ends up `-rw-------`, unlike a file created normally (`0644` under the usual umask),
and other users or tools running as another user can't read it. Fix: set the permissions on the temporary file before
the rename (for example `std::filesystem::permissions` to `0666` minus the current umask, or a plain `0644`), and add a
check to `tests/cli_export_test.cpp`. Found reviewing PR #32; to be fixed in Developer A's M2.2 work.

## Deferred

### clang-tidy

Static analysis beyond compiler warnings and sanitizers; it could also mechanically enforce one naming convention (
locals currently mix camelCase and snake_case). Noisy to set up, so it waits until M3, when performance work and
threading make it pay off. Start with a narrow check set (`bugprone-*`, `performance-*`) rather than everything.
