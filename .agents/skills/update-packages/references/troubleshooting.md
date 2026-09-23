# Troubleshooting a package update in CI

Expansion of Step 10. A package update spends most of its calendar time here. The failures
are rarely in the versioner — they are new compiler diagnostics, upstream channels moving
under the PR, and resource limits. This is the method, not a catalogue of past incidents.

## 1. Establish what is failing, before explaining anything

`gh pr checks <n>` gives the job names, their states and their URLs. Reduce that list before
touching any code:

- **Group by cause, not by job.** N red jobs are usually far fewer causes. Fixing them as
  one change, or reading them as one problem, is the standard way to waste a CI cycle.
- **Find the earliest job in each chain.** Recipes build on each other, so a build failure
  reappears as a test failure and a coverage failure downstream. Diagnose the earliest one
  and re-check the rest only after it is green.
- **Batch the fixes.** Any push restarts the whole matrix, and a force-push restarts it from
  scratch, so collect fixes and push once.

State the cause count explicitly before proposing fixes. If you cannot say how many
distinct causes there are, you are not ready to fix any of them.

## 2. Get the real error text

`WebFetch` silently truncates a CI job page and then answers from the fragment it kept,
which reads as "no failure found" for a job that plainly failed. Never conclude a job is
healthy from a truncated fetch. Download the page and search it locally instead.

Three properties of these pages shape how to read them:

- Every build step prints `Exit: <code>` in step order, so those markers alone locate the
  failing step before you read a line of output.
- The page is effectively one enormous line, so `grep -C` gives either nothing or the whole
  file. Window by character offset around each match rather than by line.
- The compiler's angle brackets arrive HTML-escaped. Strip tags first and unescape entities
  second. Unescaping first makes any tag-stripping regex eat template arguments, silently
  turning `std::vector<std::string>` into `std::vector`.

Search for compiler diagnostics (`error:`), TestHarness failures (`FAILED`), and whatever
else the failure at hand prints. Quote the diagnostic verbatim in your report — a paraphrase
hides which of several plausible causes it is.

## 3. Separate your change from upstream drift

A conda package update is exposed to channels that move independently of the PR, so a red
job is not proof the diff is wrong. Before changing anything in MOOSE, ask whether the
failing input moved:

- A solve failure naming a package MOOSE already pinned and previously built means that
  package was **rebuilt** upstream with new requirements. The build string (the `_N` suffix)
  increments on a rebuild while the version stays put.
- Query the channel for what is actually available now:

  ```bash
  curl -s https://api.anaconda.org/package/conda-forge/<pkg> \
    | python3 -c 'import json,sys; d=json.load(sys.stdin); print(d["latest_version"], d["versions"][-8:])'
  ```

- Read the failing requirement literally. It names the constraint that cannot be satisfied,
  which usually names the pin that has to move.

If the cause is upstream drift, say so in the commit message. A reviewer who thinks the PR
caused it will ask why the pin moved at all.

## 4. Use the two distributions as a bisect lever

conda recipes build with the compiler pins from `conda/conda_build_config.yaml`; apptainer
containers carry their own toolchain from their base images. When a source-level failure
appears in one distribution and not the other, the difference between their toolchains is
the first hypothesis, and it is cheap to confirm by reading both versions.

The same reasoning applies across the variant matrix: `[linux]`, `[osx]` and
`[osx and arm64]` selectors mean a pin can be satisfiable on one platform and not another,
so a failure on Mac alone is normal and points at the selectors rather than at the code.

## 5. Fallout to expect from a toolchain bump

Check for these directly rather than waiting for CI to find them:

- **New warnings promoted by `-Werror`.** Often false positives in unit tests. Decide
  explicitly whether to fix the code or to stop treating that warning as an error; do not
  silence it by accident.
- **Generated and preprocessed headers picking up new upstream constructs.** MOOSE
  preprocesses standard headers into a monolithic header for parsed-function JIT
  compilation. `-imacros` drops the scanned header's declarations from the output but keeps
  its `#pragma` directives, so a pragma newly added inside the standard library lands at
  file scope in the generated header and every JIT compile fails. Any compiler or
  libstdc++ bump can introduce one.
- **New deprecations and removals** in the standard library or in a dependency's API,
  surfacing as build failures in the modules rather than the framework.
- **Numerical differences** requiring regolds or tolerance changes, which are expected and
  belong in this PR as their own commits.

When the failure is inside a submodule that is not initialized in the worktree, read the
file at the pinned SHA rather than initializing it — an initialized submodule tree must
never be committed:

```bash
git ls-tree HEAD <submodule>                   # the pinned SHA
curl -s https://raw.githubusercontent.com/<org>/<repo>/<sha>/<path>
```

## 6. Choosing the replacement pin

The target is what `conda-forge-pinning-feedstock` pins, **not** the newest build on the
channel. The channel routinely carries versions several majors ahead of the pin; picking
one of those diverges from everything else in the channel and reintroduces the same class
of solve failure against a newer version.

```bash
curl -sL https://raw.githubusercontent.com/conda-forge/conda-forge-pinning-feedstock/main/recipe/conda_build_config.yaml \
  | grep -A6 -E '^(c_compiler_version|cxx_compiler_version|fortran_compiler_version):'
```

The three compiler keys are only the common case; substitute whichever key you are chasing,
and note that the feedstock's own keys carry `[linux]`/`[osx]` selectors, so the answer is
per-platform rather than a single version.

Within the pinned major, take the newest patch that exists for **every** pin that has to
move together, and move coupled pins in the same commit — `version-sources.md` lists which
pins mirror each other. A bump whose consequence is a tree-wide reformat or another
sweeping mechanical change belongs in its own update; record it on the `## To do` list
instead of absorbing it here.

Beware that a pin's key name is not always the package name (keys use `_`, conda packages
often use `-`). Querying the key name returns nothing and looks like "no such package";
the `meta.yaml.template` that consumes the key shows the real package name.

## 7. Landing fixes without breaking the versioner gate

`Update versioner hashes` must be the final commit, and the block it adds is keyed on
`git rev-parse HEAD` at the moment `--summary` ran — in the committed history, the commit
directly below it. **Any** commit added on top invalidates the block, even one that changes
no package hash.

To add a commit to a branch that already carries the hash commit, unwind it, commit the
work, and regenerate:

```bash
git reset --soft HEAD~                                                     # undo the hash commit
git restore --source=HEAD --staged --worktree scripts/tests/versioner_hashes.yaml
# ... commit the fix, plus its newsletter entry if it earns one ...
./scripts/versioner.py --verify upstream/devel
./scripts/versioner.py --summary   # append, add the PR number, commit last
```

To drop a hash commit that is already pushed and now sits further down the history, replay
the commits above it onto its parent:

```bash
git rebase --onto <hash-commit>^ <hash-commit> <branch-name>
```

Pass the **branch name** as the last argument. Passing a SHA replays the commits correctly
but leaves the branch pointer behind and lands you on a detached HEAD; recover with
`git checkout -B <branch-name> <new-head>`. Either way the branch then diverges from the
remote, so publishing needs `git push --force-with-lease origin <branch-name>` and the
user's approval.

A late commit that touches an influential file usually needs **no** further version bump:
the invariant is measured against the base ref, and the version sweep already moved that
package's `full_version` away from it, so `--verify` reports `CHANGE`. Bump again only when
`--verify` actually says `NEED BUMP` — the case where the package was not in the original
sweep and its version still matches the base ref. Let `--verify` decide; do not
pre-emptively bump on the theory that a changed file must mean a changed version.

## 8. Re-read what the fix falsified

`--verify` checks versions, not claims. A fix late in the PR can contradict something the
newsletter or the PR body already asserts — most often a paragraph explaining why a pin was
deliberately left where it was, which stops being true the moment the pin moves. CI cannot
catch this. After each fix, re-read both documents for the rationale you just invalidated,
and add the bullets the fix earns.

## 9. Know what not to fix

- **Deferred failures** go on the `## To do` checklist with enough detail to resume from
  cold: the diagnostic, the mechanism, and where it fires. A deferred item with no
  reproduction path costs the next person the whole diagnosis again.
- **Ask before fixing anything the user has not scoped.** Compiler fallout can range from a
  one-line test change to a framework change with its own review; which of those belongs in
  a package update is the user's call, not yours.
- **Permission failures are not engineering problems.** Applying a label can fail outright
  without triage or write access on the repository. Note it, hand it to a maintainer, and
  move on. When such a failure comes from a command that did several things at once, check
  what actually landed rather than assuming the whole command was lost.
