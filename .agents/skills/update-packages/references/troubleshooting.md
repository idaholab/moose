# Troubleshooting a package update in CI

Expansion of Step 10 (Iterate) in `../SKILL.md`. A package update spends most of its
calendar time here. The failures are rarely in the versioner — they are new compiler
diagnostics, upstream channels moving under the PR, and resource limits.

## 1. Find and route the failures

Use `civet-ci-failures` to list the failing jobs, separate causes from dependency cascades,
read the error text, and get each failure's reproduce command. Then route each cause:

| Signature | Section |
|---|---|
| conda solve failure naming a pinned package | 2 |
| compile `error:` | 6 |
| `TIMEOUT` that repeats on retry, or a crash | 8 |
| `CSVDiff`/`Exodiff`/`SCHEMADIFF` | 9 |

## 2. Separate your change from upstream drift

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

## 3. Trace the versions a submodule bump drags in

A submodule bump moves more than its own code. PETSc is the main case: `configure_petsc.sh`
passes `--download-openblas`, `--download-mumps`, `--download-hypre` and others without a
version, so each one takes the default in the PETSc tag's
`config/BuildSystem/config/packages/<Name>.py`. Moving the `petsc` pointer silently moves
all of them. Only an explicit `--download-<pkg>-commit=...` in `configure_petsc.sh` holds
one back (Kokkos is the standing example).

Compare the defaults between the old and new tag without cloning the history. A
blob-filtered shallow fetch of the two tags is enough:

```bash
d=$(mktemp -d); cd "$d" && git init -q
git remote add o https://gitlab.com/petsc/petsc.git
git fetch -q --depth 1 --filter=blob:none o tag v<old> tag v<new>
for v in v<old> v<new>; do
  git show $v:config/BuildSystem/config/packages/OpenBLAS.py | grep -E 'self.version|gitcommit'
done
git diff v<old> v<new> -- config/BuildSystem/config/packages/MUMPS.py
```

File names are case-sensitive and not uniform: `OpenBLAS.py`, `MUMPS.py`, `HDF5.py`,
`SuperLU_DIST.py`, `hypre.py`, `kokkos.py`, `kokkos-kernels.py`. List the directory
with `git ls-tree --name-only` before concluding a file does not exist. A raw-file URL to
GitLab can return nothing while a fetch works.

The same file answers whether an older version is still accepted: `self.minversion`, if
present, is the floor, and `--download-<pkg>-commit=<ref>` is a generic option every package
supports. Read the build command line in the same file too; it decides things like the
library's threading model (PETSc builds OpenBLAS with OpenMP whenever OpenMP is found).

Then read the upstream project's changelog for the versions in between and search it for
the failure's terms: the routine named in the error, `NaN`, `thread`, `lock`. A release
that adds input validation or reworks threading can explain a failure outright.

The same applies to anything else that vendors or bundles sources (pytorch carries its own
protobuf and ONNX, for example). When a component is not checked out in the worktree, read
it from the upstream tag, or download the conda package and inspect its files.

## 4. Compare against what is already fixed upstream

Before writing a fix, check whether `next` already has one, and whether the failing run
included it:

```bash
git fetch upstream next devel
git log --oneline HEAD..upstream/next -- <file>          # changes not on the branch
git diff HEAD upstream/next -- <file>
git merge-base --is-ancestor <fix-commit> <CIVET_BASE_SHA> && echo included || echo "not included"
```

A fix merged into `next` after the failing job's `CIVET_BASE_SHA` will be in the next run,
because a new push creates a new Civet event against the current `next`. Nothing is needed
in this PR. A fix that is in `next` but not `devel` is not reached by rebasing onto `devel`.

When a reviewer says a failure will go away after rebasing, verify it rather than assume it.
Apply their commits to the worktree without committing, rebuild, rerun the reproducer, and
restore the files afterwards:

```bash
git cherry-pick --no-commit <first>^..<last>
# rebuild and rerun
git restore --staged --worktree <files the commits touched>
```

Confirm the rebuild picked up the change before trusting the rerun: the rebuilt library
must be newer than the edited source. A stale binary tests the old code and makes the
result meaningless.

Check each step's exit status, not just that its output file exists. A failing compiler or
preprocessor can leave an empty or partial file behind, and `make` then treats that file
as up to date, so later steps quietly test the wrong thing.

## 5. Use the two distributions as a bisect lever

conda recipes build with the compiler pins from `conda/conda_build_config.yaml`; apptainer
containers carry their own toolchain from their base images. When a source-level failure
appears in one distribution and not the other, the difference between their toolchains is
the first hypothesis, and it is cheap to confirm by reading both versions.

A failure confined to one distribution can also come from a code path only that
distribution enables, not from its toolchain. The conda recipes, for example, set
environment variables in `conda/<pkg>/build.sh` that switch MOOSE onto installed-package
code paths the source-tree builds never take. `git grep` the variable or file named in
the error across `conda/` and `apptainer/` to see which distributions reach it.

The same reasoning applies across the variant matrix: `[linux]`, `[osx]` and
`[osx and arm64]` selectors mean a pin can be satisfiable on one platform and not another,
so a failure on Mac alone is normal and points at the selectors rather than at the code.

## 6. Fallout to expect from a toolchain bump

Check for these directly rather than waiting for CI to find them:

- **New warnings promoted by `-Werror`.** These usually flag real defects, so fix the code.
  Stop treating a warning as an error only after showing it is a false positive, and do so
  explicitly.
- **Generated and preprocessed headers picking up new upstream constructs.** MOOSE
  preprocesses standard headers into a monolithic header for parsed-function JIT
  compilation. `-imacros` drops the scanned header's declarations from the output but keeps
  its `#pragma` directives, so a pragma newly added inside the standard library lands at
  file scope in the generated header and every JIT compile fails. Any compiler or
  libstdc++ bump can introduce one.
- **New deprecations and removals** in the standard library or in a dependency's API.
- **Numerical differences.** Follow `brittle-numerics-root-cause` to decide between a regold,
  a tolerance change, and a code change that removes the sensitivity. Each fix belongs in
  this PR as its own commit.

When the failure is inside a submodule that is not initialized in the worktree, read the
file at the pinned SHA rather than initializing it — an initialized submodule tree must
never be committed:

```bash
git ls-tree HEAD <submodule>                   # the pinned SHA
curl -s https://raw.githubusercontent.com/<org>/<repo>/<sha>/<path>
```

A compiler package can also change behaviour between builds without a version change in
the code that uses it, for instance by adding a default configuration file. conda-forge
clang's `default_cfg` builds ship a `<triple>-clang++.cfg` that adds
`-isystem $CONDA_PREFIX/include` ahead of every command-line include directory, so an
environment library can shadow a vendored copy of an older version of the same library.
An error note pointing at `<prefix>/bin/../include`, or a `Configuration file:` line in the
job, is the sign; read the `.cfg` from the conda package itself.

Prove a flag or include-order fix with the smallest program that would show it working
before shipping it, because compilers silently discard some combinations. GCC and clang, for
one, drop a `-I` directory that is also given with `-isystem`, so adding `-I` for a
directory a build already passes as a system directory does nothing. To test an include
order, put a header of the same name in each directory, have each define a different string,
and compile a program that prints it.

## 7. Reproduce locally in the PR's own environment

Civet is slow to iterate on. When a failure needs more than reading logs, reproduce it in the
environment the job used.

For apptainer jobs, every PR publishes its containers. Get the exact URI from the generator
rather than composing it by hand:

```bash
./scripts/apptainer_generator.py uri moose-dev --suffix <os>-<compiler>-<mpi> --tag pr-<n>
./scripts/apptainer_generator.py uri moose-dev --suffix <os>-<compiler>-<mpi> --tag pr-<n> --check
```

Then pull it, build the minimum needed inside it, and run the failing input directly:

```bash
apptainer pull pr.sif <uri printed above>
apptainer shell -B <moose> pr.sif
cd <moose> && ./configure <the job's configure options> && cd test && make -j 16
```

The container carries the dependency stack (PETSc, libMesh and whatever else it was built
with), so the submodules are not used. Copy the job's `Running command:` line and the
environment the TestHarness printed for that test exactly; a reproduction that differs in
one variable proves nothing.

For conda jobs, the equivalent is an environment created from the PR's packages, when Civet
has published them, or from the same channel state the job resolved.

Large downloads, long builds and long test runs are the user's call. Offer copy-paste steps
they can run themselves, with what each result means, and ask before running them yourself.
AGENTS.md section 6 still applies: settle the conda question before building.

## 8. Isolate a hang or crash

**Measure how far the failure reaches before choosing a fix.** Search every job in the
matrix for the same signature, not only the job where it was first seen: for a hang, every
`TIMEOUT` in every threaded step, across the framework and all modules. Group the hits by
what they share (the direct solver named in their output, the library on the stack, the
build mode). A cause narrowed down on one test can turn out to be a library defect that a
whole class of tests hits, and a fix in one caller cannot cover that.

**Run each candidate cause as one row of a matrix.** Change one variable per row and wrap
every run in `timeout`, so a hang becomes exit code 124 instead of a stuck terminal. For a
threading hang, the variables are each separate source of a thread count: the application's
`--n-threads`, `OMP_NUM_THREADS`, and any library's own variable. For example:

```bash
OMP_NUM_THREADS=2                        timeout 120 $EXE -i $IN --n-threads=2 > A.log 2>&1; echo "A $?"
OMP_NUM_THREADS=2 OPENBLAS_NUM_THREADS=1 timeout 120 $EXE -i $IN --n-threads=2 > B.log 2>&1; echo "B $?"
OMP_NUM_THREADS=1                        timeout 120 $EXE -i $IN --n-threads=2 > C.log 2>&1; echo "C $?"
OMP_NUM_THREADS=2                        timeout 120 $EXE -i $IN               > D.log 2>&1; echo "D $?"
```

Choose rows that separate the hypotheses: here B isolates the BLAS library's threads, C the
OpenMP setting, and D removes `--n-threads` entirely. Record the results as a table. A single
wrong exit code reverses the conclusion, so confirm each value when the user reports it.

When giving the user commands to run, copy paths and names from one source rather than
retyping them, and prefer checks that do not depend on timing; a check that races the
process it inspects fails exactly when the fix works.

**Take a backtrace of every thread.** A matrix says which variable matters, and a backtrace
says where the code waits:

```bash
$EXE -i $IN --n-threads=2 > /dev/null 2>&1 & pid=$!
sleep 30
gdb -batch -ex "info threads" -ex "thread apply all bt" -p $pid > gdb.txt 2>&1
kill $pid
```

Write the whole output to a file. `tail` cuts off the innermost frames and the other
threads, which are the frames that matter. Identify each thread and set aside the ones that
are always present (MPI progress threads in `epoll_wait`, MOOSE's live perf-graph printer).
Then note what is absent as well as present: a thread waiting for workers when no worker
thread exists means the thread team was smaller than the work was split for, not that a
worker is stuck.

**Swap one library under the same binary.** To test whether an older version of a library
fixes the failure, build that version with the same options in the same environment and
preload it, instead of rebuilding the whole stack:

```bash
LD_PRELOAD=<old>/libfoo.so.0 timeout 120 $EXE -i $IN ...   # expect pass
timeout 120 $EXE -i $IN ...                                 # control: expect the failure
```

Confirm the preload actually replaced the library. Ask the loader, which needs no running
process; only the preloaded copy should appear:

```bash
LD_PRELOAD=<old>/libfoo.so.0 ldd $EXE | grep libfoo
```

Reading `/proc/<pid>/maps` of a running process also works, but only while it runs: a
fixed run often finishes before the check, and the file is then gone. Also check that the
variable holding the path is set in the current shell; an empty `LD_PRELOAD` silently runs
the original library.

Build the old version with the options the stack uses (Section 3). When the installed PETSc
keeps no `configure.log`, take the compilers from
`$PETSC_DIR/lib/petsc/conf/petscvariables` and the threading model from the library's
installed config header. Match every option, including the threading model, integer size
and CPU architecture targets; the targets select different kernels, which round
differently.

## 9. Size a numerical diff before regolding

A diff is evidence of a cause only once its size is known. Extract the changed values, and
report the relative change against the test's `rel_err` (and `abs_zero`). Separate values
near zero, where large relative changes mean nothing, from values near the field's maximum.

Then look for the library that moved under the solve, often one PETSc downloads (Section 3),
and follow `brittle-numerics-root-cause` to choose the fix.

## 10. Choosing a replacement pin or other fix

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

When the culprit is a dependency that a submodule downloads (Section 3), there is usually
more than one way to resolve it, and which one fits depends on the case:

- hold that dependency at a good version with `--download-<pkg>-commit=<ref>` in
  `scripts/configure_petsc.sh` (the existing Kokkos pin is an example);
- change how it is built (a `--download-<pkg>-*` option, a different threading model);
- move to a newer upstream release or commit that already contains a fix;
- change the MOOSE, libMesh or application code that triggers the problem;
- work around it in the test inputs or the TestHarness, when the failure is confined there.

Lay these out with their trade-offs and **confirm the choice with the developers who own the
affected code before implementing any of them.** Do not treat a pin as the default answer: a
pin holds back fixes, diverges from what PETSc expects, and needs someone to remove it later,
and the owners may prefer a fix in their own code. `git log -- scripts/configure_petsc.sh`
shows what has been done before, which informs the discussion but does not decide it.

Whichever option is chosen, check that it is accepted by the new submodule version and
prove locally that it fixes the failure (Section 8) before pushing it. For a pin, choose the
**newest** good release, not just a known good one: test the release right before the bad
one first, since an older pin drops fixes for no reason. Give the change a comment that says
why, add a newsletter line, and put any follow-up ("drop the pin once upstream is fixed") on
the `## To do` list.

## 11. Landing fixes without breaking the versioner gate

`Update versioner hashes` must be the final commit, and the block it adds is keyed on
`git rev-parse HEAD` at the moment `--summary` ran — in the committed history, the commit
directly below it.

Know which of those two facts CI actually enforces, because it decides how much work each
cycle costs. `test_versioner.py` loads the file and walks the entries it finds, resolving
each key with `get_packages(<commit>)`; nothing asserts that the branch tip has an entry.
So a commit added on top costs nothing — the keyed commit is immutable and still reachable,
and its recomputed hashes are unchanged. What fails is an unresolvable key: an amend,
reset or rebase that orphans the keyed SHA makes the test raise
`Reference <sha> is not valid` in a fresh clone of the branch. "Keep it last" is therefore a
review expectation about recording the final state, while "never orphan its key" is the
hard constraint.

The cheap way through Step 10 is to carry no hash commit at all: drop it at the first fix
with the `git rebase --onto` recipe below, push fixes as plain fast-forwards, and re-add the
block once the branch is green. Regenerating it per cycle instead costs a `reset --soft` and
a force-push every time, and a force-push restarts the matrix from scratch.

When you do need to add a commit beneath an existing hash commit — regenerating the block at
the end, or amending it to carry the PR number — unwind it, commit the work, and regenerate:

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

## 12. Satisfy the Precheck format and lint gates before pushing

`Precheck` runs the formatters and linters, and it gates the whole matrix: when it fails,
every other job stays `Not_Started` and nothing else in the PR is tested that cycle. Its
three format steps are not allowed to fail, unlike `Size check`, `Fixup commit check` and
`Submodule check`, so one unformatted line costs a full cycle. Run all three before pushing:

```bash
git clang-format upstream/devel                   # C and C++, diff-scoped
git clang-format --extensions=k upstream/devel    # Kokkos (.K), diff-scoped
black --check --diff --config pyproject.toml .    # Python, whole tree
ruff check --no-cache --config pyproject.toml .   # Python, whole tree
```

Those are Civet's own invocations; black and ruff are separate `Precheck` steps. Drop
`--check --diff` from black, or add `--fix` to ruff, to apply what they report instead of
printing it.

What is in scope differs per tool, which is what makes "my edit was small" an unreliable
guide:

- **`git clang-format` is diff-scoped** and reformats only the lines the diff touches. Its
  default extension list covers `.C`/`.h` and plain `.c`, so a C file is gated exactly like a
  C++ one; Kokkos `.K` files need the second, `--extensions=k` run.
- **black runs over the whole tree**, minus the `extend-exclude` list in `[tool.black]`
  (contrib, submodules, `petsc`, `libmesh`). Every other Python file is in scope whether or
  not this PR touched it, so a file that arrives unformatted from elsewhere fails the gate
  here.
- **ruff runs over the whole tree, but `[tool.ruff] include` is an allowlist** — currently
  parts of `python/` and `scripts/coverage.py`. Most Python in the repo is therefore
  unlinted, and black being clean says nothing about ruff. Inside the allowlist the enabled
  rules (`D`, `E`, `F`, `I`, `N801`, `SIM`, `TID`) require NumPy-style docstrings and sorted
  imports, which is stricter than the rest of the tree looks.

Do not hand-format on the theory that the result looks like the surrounding code.
`.clang-format` sets `ColumnLimit: 100`, and clang-format keeps an initializer on one line
when it fits in exactly 100 columns while wrapping one that is a single character longer, so
two adjacent declarations that read as a matched pair are formatted differently.

**These tools are themselves pins in this update.** `black`, `ruff`, `clang_format` and
`clang_tools` all live in `conda/tools/conda_build_config.yaml`, mirrored in
`requirements.txt` (`version-sources.md` has the row). That cuts both ways: a binary from an
older environment can disagree with the one Civet runs, so check versions before trusting a
clean local run; and moving one of those pins changes what the gate accepts, so a tree that
was clean can go red without anyone editing it. A bump whose consequence is a tree-wide
reformat or relint belongs in its own update — the same rule `conda/conda_build_config.yaml`
already states for a clang-format major bump.

`scripts/install_format_hook.sh` installs a pre-commit hook that runs `git clang-format` on
staged C++ files under `framework`, `modules`, `test`, `unit`, `examples`, `tutorials` and
`stork`. It does not cover Python; black and ruff stay manual.

When `Precheck` has already failed on formatting, take its answer instead of guessing again.
The `Clang format` step publishes the patch and prints the command:

```bash
curl -s https://mooseframework.inl.gov/docs/PRs/<pr>/clang_format/style.patch | git apply -v
```

Fold the formatting into the commit that introduced the code rather than adding a follow-up
commit, and regenerate the hash block afterwards — the amend invalidates it, as above.

## 13. Re-read what the fix falsified

`--verify` checks versions, not claims. A fix late in the PR can contradict something the
newsletter or the PR body already asserts — most often a paragraph explaining why a pin was
deliberately left where it was, which stops being true the moment the pin moves. CI cannot
catch this. After each fix, re-read both documents for the rationale you just invalidated,
and add the bullets the fix earns.

## 14. Know what not to fix

- **Deferred failures** go on the `## To do` checklist with enough detail to resume from
  cold: the diagnostic, the mechanism, and where it fires. A deferred item with no
  reproduction path costs the next person the whole diagnosis again.
- **Ask before fixing anything the user has not scoped.** Compiler fallout can range from a
  one-line test change to a framework change with its own review; which of those belongs in
  a package update is the user's call, not yours.
- **A finished diagnosis is not a decided fix.** A fix that worked for one failure is not the
  rule for the next one that looks similar. When several fixes are possible, or the fix
  touches code that other developers own, present the options and check with those
  developers before implementing. Record the item as "waiting on <who>" in notes and on the
  `## To do` list, so it is not picked up as ready to implement in a later session.
- **Prove a fix before pushing it.** A fix reasoned out but never run (a compiler flag, an
  include-order change, a pin) costs a full CI cycle when it is wrong. Run the smallest
  check that would show it working, and check the repository's history for how the same
  file has been changed before, before proposing it.
- **Permission failures are not engineering problems.** Applying a label can fail outright
  without triage or write access on the repository. Note it, hand it to a maintainer, and
  move on. When such a failure comes from a command that did several things at once, check
  what actually landed rather than assuming the whole command was lost.
