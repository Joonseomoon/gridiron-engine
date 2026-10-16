# Working Together

## One feature and two connected contributions

Developer A and Developer B are temporary planning labels, not permanent subsystem owners. Both write product functionality and tests. Rotate implementation areas within milestones as shown in ROADMAP.md. Either person can request a switch at a round boundary; preserve the planned crossover rather than fixing one person to a specialty.

Before splitting work, agree on the shared inputs, outputs, assumptions, and one demo. Each person's contribution must connect to the other person's work during the milestone, not only at its end.

## Milestone rhythm

1. Together, define the smallest end-to-end demo and acceptance criteria.
2. Agree on the shared data and interfaces; write down one example.
3. Create two parallel task groups for the next round, with owners, explicit prerequisites, and a shared checkpoint. Balance their scope before starting.
4. Each developer builds a nearby piece, including appropriate tests.
5. Integrate the smallest working path early, ideally during the first shared work session.
6. Review each other's small pull requests, integrate, and rotate areas for the next round.
7. Run the finished demo together and record limitations before moving on.

Sample data or stub responses can unblock work briefly. Do not build an entire milestone against an untested interface. If either person is blocked by the other, pair on the dependency or split it into a smaller shared change.

## Parallel work and rotation

Parallel work means each person can begin after the shared prerequisite is ready. An agreed example state, result, or dataset can support independent work. Do not label two tasks parallel when one needs the other's unfinished output and has no usable baseline.

Keep interface changes small and agreed in advance. Assign separate implementation files where practical. Build shared setup together, then split; use short integration checkpoints instead of waiting until the milestone ends. If a dependency appears, pair to unblock it or re-split the current round. Do not fill waiting time by sending one person far ahead in the roadmap.

A switch includes a short walkthrough and a meaningful implementation task for the new lead. The previous lead reviews and supports; they do not retain exclusive ownership. Each author owns relevant tests and documentation. Reviewing alone does not count as hands-on experience.

Maintain [the learning record](docs/LEARNING.md) at milestone boundaries. Aim for breadth over the project, not equal minutes or duplicate implementations. Adjust future assignments for interests and gaps.

## Principles

- Keep main working and merge small changes frequently.
- Establish correctness before optimization.
- Separate football rules from probability models.
- Record assumptions, seeds, model versions, and evaluation settings.
- More simulations reduce sampling noise; they do not fix a poor model.
- Treat market analytics as research, with no promise of betting profit.

## Branches and review

- Work in the same repository using short-lived feature branches.
- Keep each pull request focused on one understandable behavior.
- Open a draft pull request early when changing shared interfaces.
- The other developer reviews before merging to main.
- Sync with main frequently and coordinate changes to shared files.
- Avoid reserving all engine work for one person or all tests for the other.
- After the initial documentation commit, use this review workflow for project changes.

Follow these rules even if GitHub does not enforce them on the repository's plan.

## Shared understanding

At integration checkpoints, each developer explains their change and the other traces how it connects to the whole feature. Both should be able to run and troubleshoot the demo. Review helps share understanding; it does not require both people to edit every file.

## Definition of done

A change works, its meaningful checks pass, the other developer has reviewed it, and relevant documentation is updated. A milestone is complete when both developers can run the agreed demo from a fresh checkout and explain its limitations.

## Issue format

- Goal and visible behavior
- Primary owner
- Prerequisites already available
- Shared interface and example/baseline that permits parallel work
- Learning area and planned next-round crossover
- Acceptance criteria
- Integration checkpoint

## Pull request format

Use the included template. Describe behavior, validation, and how this connects to the other developer's work. Keep planning lightweight.
