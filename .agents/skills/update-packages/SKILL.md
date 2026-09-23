---
name: update-packages
description: >-
  Carry out a MOOSE distributed-package update: bump submodules and pinned upstream
  versions, bump package versions and build numbers in scripts/versioner.yaml, rebuild
  the conda templates, verify with scripts/versioner.py, write the newsletter entries,
  append the versioner hashes, and open the "PR: Updates packages" pull request against
  next. Use when someone asks to update the conda or apptainer packages, do the monthly
  or mid-month package update, bump petsc/libmesh/vtk/wasp/mfem/neml2/pytorch/tools
  versions, refresh the conda-forge pins or base container images, or diagnose a failing
  `versioner.py --verify`.
---

# Updating MOOSE distributed packages

MOOSE ships its dependency stack two ways from one source of truth:

- **conda** recipes under `conda/` (built by Civet, published to `conda.software.inl.gov/public`)
- **apptainer** containers under `apptainer/` (built by Civet, published to a registry)

Both are versioned by `scripts/versioner.yaml` and driven by `scripts/versioner.py`.
The authoritative human doc is the wiki page
[Updating packages](https://github.com/idaholab/moose/wiki/Updating-packages); this
skill is the operational expansion of it, including the parts the wiki leaves implicit
(where upstream versions are pinned, what the newsletter and PR body must contain, and
how to recover when `--verify` fails).

A package update is a *large, iterative, externally visible* change. It routinely takes
dozens of commits and several Civet cycles. Do not treat it as a one-shot task, and do
not push or amend without approval — see `pr-create` for the git/publication approval
rules, which apply here too.

## Ground rules

1. **Never hand-edit a generated file.** Every value in a `templates:` map in
   `scripts/versioner.yaml` is generated. In practice: every `conda/<pkg>/meta.yaml`
   that has a `meta.yaml.template` sibling. Edit the `.template`, then regenerate.
   `conda/conda_build_config.yaml`, `conda/build/conda_build_config.yaml` and
   `conda/tools/conda_build_config.yaml` are *not* generated — edit those directly.
2. **Version each thing in exactly one place.** Several versions are mirrored
   (conda ↔ apptainer ↔ `requirements.txt`). `references/version-sources.md` lists every
   pin, its single source of truth, and its required mirrors.
3. **The versioner-hash commit is always last.** Any later amend, rebase, or new commit
   invalidates it and it must be regenerated. This is the single most common way a
   package-update PR fails CI.
4. **`--verify` clean is the gate**, not "it looks right". Run it against `upstream/devel`
   — the same ref every step of this task uses, and the one the published packages
   correspond to — not against the PR's base branch `next`, which already contains `devel`
   plus whatever else is queued. Run it again after every history change.
5. **Bump dependers.** Changing a package's influential files changes its hash, which
   changes the hash of everything downstream. `scripts/versioner.yaml` records
   `# dependers: ...` in a comment above each package — trust the comment as a map, but
   let `--verify` be the authority on what still needs a bump.
6. **Every commit references the PR or issue.** End the message body with
   `refs idaholab#<pr/issue>`, above any `Co-Authored-By:` line. A package update is dozens
   of commits across several Civet cycles, so the trailer is what ties a late compiler fix
   back to the update it belongs to. Add it when writing the commit; going back to amend a
   pushed history for a missing trailer costs a force-push and a full CI restart.

   The number is not known until Step 9 creates the PR. Either open the PR early — a draft
   is enough to get a number — or reference the tracking issue for the update if one
   exists. Most of the upstream history carries the bare `refs #<n>` form; `idaholab#<n>`
   resolves the same on GitHub and stays unambiguous when the branch lives on a fork.

## Step 0 — Establish the working state

1. Confirm the repo root, and that `git status` is clean. Work on a dedicated branch off
   `upstream/devel` (the historical naming is descriptive, e.g. `packages_sept26`,
   `jun26_packages`, `package_update_226`).
2. Fetch `upstream` and record the merge base. Every `--verify` in this task uses
   `upstream/devel`.
3. Confirm `scripts/versioner.py` runs at all before changing anything:

   ```bash
   ./scripts/versioner.py --verify upstream/devel
   ```

   On a fresh branch this must report `Verification succeeded.` with everything `OK`. If
   it does not, `upstream/devel` itself is broken — say so and stop rather than absorbing
   someone else's failure into this PR.
4. Ask the user for the intended scope if it was not given. A "monthly update" is not a
   fixed set; the scope is a decision. Offer the standard sweep from
   `references/version-sources.md` and let them subtract.

## Step 1 — Survey what is available upstream

Work through `references/version-sources.md` and produce a table of *current pin* →
*available upstream* → *proposed new pin* for every item in scope. Present this table
and get agreement before touching any file. Do not silently pick "latest" for anything
that has a release cadence (PETSc, VTK, conda-forge pins) — latest-master and
latest-release are different decisions.

Two items deserve explicit confirmation because they drop platform support or require
a coordinated change elsewhere:

- **Base container images** (`apptainer/mpi.def` `FROM_IMAGES`) come from the separate
  `idaholab/moose-containers` repository. New digests must exist there *first*. A CUDA
  or compiler bump here can remove support for specific INL hardware — call that out.
- **Compiler and system pins** (`conda/conda_build_config.yaml`,
  `conda/build/conda_build_config.yaml`) track `conda-forge-pinning-feedstock`. Moving
  them can break the MPI stack on one platform only; expect Mac-specific fallout.

## Step 2 — Land the source changes

Make the actual content changes, in small commits, each describing one thing:
`Update petsc to v3.25.4`, `Update libmesh submodule`, `Update code-server to 4.132.0`,
`Bump versions to match conda-forge-pinning-feedstock`. This history is read during
review and is the raw material for the newsletter and PR body, so keep messages specific
about *what moved from where to where*.

Do not bump anything in `scripts/versioner.yaml` yet, and do not regenerate templates
yet. Keep this phase purely about content.

For submodules, update the pointer only — do not commit an initialized submodule tree or
a `-dirty` pointer. Verify with `git diff --submodule=short`.

## Step 3 — Bump versions in `scripts/versioner.yaml`

Apply the rules in `references/versioner.md`, which cover:

- which packages use date versions, upstream versions, or `date_sha` versions
- when to bump `version` vs. `build_number`, and when `build_number` must reset to 0
- how to fan a bump out to dependers
- the `__VERSIONER_*__` template-variable/dependency constraint

Commit this as its own commit naming the packages touched, e.g.
`Update packages` or `Bump versions of tools, libmesh, petsc and moose-dev`.

## Step 4 — Regenerate the templated files

```bash
./scripts/versioner.py --build-templates
```

It prints `MODIFIED`/`UNCHANGED` per generated file. Review the diff — it should contain
version and build-number changes only. Then fold it into the Step 3 commit:

```bash
git add <the modified conda files>
git commit --amend --no-edit
```

If `--build-templates` errors with `Unused template variable __VERSIONER_X_...__`, the
template references a package that is not the package itself nor in its `dependencies:`
list in `scripts/versioner.yaml`. Fix the dependency list, not the template.

## Step 5 — Verify

```bash
./scripts/versioner.py --verify upstream/devel
```

Three tables come back: templates, influential files, versions. Read
`references/versioner.md#interpreting---verify` for every status value. The short form:

- Template `BEHIND` → redo Step 4.
- Version `NEED BUMP` → a package's hash changed but its full version did not; bump it.
- `DATE DECREASE` / `FUTURE DATE` / `BUILD NONZERO` → the version string itself is wrong.
- Influential `NEW`/`REMOVED` are informational, but a `NEW` influential file that you
  did not intend to add means `versioner.yaml`'s `influential:` list is now wrong.

Iterate Steps 3–5 until it prints `Verification succeeded.` with no red. A
`WARNING: You have changes not yet committed` line means the output is untrustworthy —
commit first, then re-verify.

## Step 6 — Write the newsletter entries

Package updates are user-visible and are documented in
`modules/doc/content/newsletter/<YYYY>/<YYYY>_<MM>.md` for the month the PR merges.
Fill in the existing `## libMesh-level Changes`, `## PETSc-level Changes`,
`## Conda Package Updates`, and `## Apptainer Package Updates` headings.

Exact structure, heading levels, and bullet phrasing conventions (including the
"Trigger rebuild; no package changes" idiom and the compare-link format) are in
`references/newsletter-and-pr.md`. Two rules worth stating up front:

- There is **one `###` block per package per update**, titled with the *new full
  version* exactly as `versioner.py <pkg>` reports it. A second update in the same month
  appends a second set of blocks rather than editing the first.
- Every package whose version was bumped gets a block, including ones that only moved
  because a dependency did.

Commit as `Update newsletter with package changes`.

## Step 7 — Append the versioner hashes

Only once Step 5 is clean and Steps 2–6 are committed:

```bash
./scripts/versioner.py --summary
```

Append the YAML block to the end of `scripts/tests/versioner_hashes.yaml`, separated from
the previous entry by a blank line, and put the PR number as a comment next to the
40-character commit hash (add it once the PR exists; see Step 9). The `app` package is
intentionally absent. Commit as `Update versioner hashes`, and keep it last.

This file is what `scripts/tests/test_versioner.py` checks, so it is how the repo proves
the versioner still computes historical hashes correctly.

## Step 8 — Validate locally

```bash
cd scripts && ./run_tests --re versioner
```

This runs `test_versioner.py`, which walks every entry in `versioner_hashes.yaml`; it is
slow and spawns processes. It is the check that most reliably catches a stale hash commit.

AGENTS.md section 6 requires asking the user whether their stack uses conda, and which
environment to activate, *before* running this. Ask early — at Step 0, alongside the scope
question — rather than at the end, so the answer is in hand when you get here instead of
blocking the last check in the task.

It needs an activated `moose` conda environment: the TestHarness imports `hit` (built
pyhit) and the test itself imports `mooseutils` and `mock`. If `hit` is unavailable but
the conda environment is, run the unittest directly instead:

```bash
PYTHONPATH=$PWD/python python3 scripts/tests/test_versioner.py
```

If neither works in the current environment, say so plainly rather than reporting the
check as passed — `versioner.py --verify` is not a substitute for it.

Also worth running before pushing, because they are cheap and they are what a reviewer
will notice:

- `./scripts/versioner.py <pkg> --yaml` for each bumped package, to eyeball the computed
  full version and influential set. This needs no conda environment beyond `yaml`,
  `jinja2` and `tabulate`.
- `cd scripts && ./run_tests --re premake` if `versioner.yaml` structure changed.
- The three `Precheck` format gates, which gate every other job in the matrix, so one
  unformatted line costs a whole CI cycle. `git clang-format upstream/devel` for C and C++;
  `black --check --diff --config pyproject.toml .` and
  `ruff check --no-cache --config pyproject.toml .` for Python. The Python pair runs over the
  whole tree rather than the diff, so run them even when the update touched no `.py` file.
  Do not hand-format instead — see `references/troubleshooting.md`.

Full conda and apptainer builds are Civet's job, not something to attempt locally unless
the user asks. If they do, `conda/generate_recipe.sh` and
`scripts/apptainer_generator.py <library> build` are the entry points.

## Step 9 — Open the PR

Follow `pr-create` for the audit and the approval-gated push. Package-update specifics:

- Base is `next`.
- Apply the `PR: Updates packages` label (`gh pr edit <n> --add-label "PR: Updates packages"`).
- The body is not the standard PR template. It mirrors the newsletter, plus a `## To do`
  checklist tracking the coordination work that has to happen before merge — waiting on
  an upstream release, a companion PR in `moose-containers` or `libMesh`, enabling
  optional conda recipes on merge. See `references/newsletter-and-pr.md` for the layout.
- Once the PR number exists, add it as the comment on the hash entry from Step 7 and
  amend that last commit.

## Step 10 — Iterate

Expect Civet failures, and expect to spend more time here than on Steps 1-9 combined.
Package updates routinely require regolding, tolerance changes, and small source fixes for
new compiler or library behavior — those belong in this same PR, as their own commits, and
often need a newsletter mention too.

`references/troubleshooting.md` is the playbook for this phase: reducing a list of red jobs
to a list of causes, reading a job page without being misled by a truncated fetch, telling
your diff apart from a channel that moved underneath the PR, what a toolchain bump
predictably breaks, and the git recipes for landing a fix without invalidating the hash
commit.

After **any** history change (new commit, amend, rebase onto `next`):

1. Re-run `./scripts/versioner.py --verify upstream/devel`.
2. Re-run `./scripts/versioner.py --summary` and replace the last block in
   `scripts/tests/versioner_hashes.yaml`.
3. Keep the hash commit last.

Step 2 is not optional bookkeeping: the block is keyed on `HEAD` at the moment `--summary`
ran, so any commit added above it invalidates it even when no package hash moved.

A late commit that touches an influential file usually needs **no** further version bump.
The invariant is measured against the base ref, and Step 3 already moved that package's
`full_version` away from it, so `--verify` reports `CHANGE` and there is nothing to do.
Bump again only when `--verify` actually says `NEED BUMP` — the case where the package was
not part of the original sweep and its version still matches the base ref.

`--verify` cannot see prose. A fix can falsify something the newsletter or the PR body
already claims, so re-read both after each one.

## When a package update is already in flight and broken

First decide which kind of broken it is. If the versioner gate itself is red, work the list
below. If the versioner gate is clean and the red jobs are builds and tests, the failures
are content failures and `references/troubleshooting.md` is the relevant reference — do not
go looking for a versioner problem that `--verify` has already ruled out.

Diagnose in this order, since each step's answer changes the next:

1. `git log --oneline upstream/devel..HEAD` — is `Update versioner hashes` the last commit?
2. `./scripts/versioner.py --verify upstream/devel --brief` — which table fails?
3. `cd scripts && ./run_tests --re versioner` — does the failure reproduce the CI failure?
4. Compare `./scripts/versioner.py --summary` against the tail of
   `scripts/tests/versioner_hashes.yaml`.

Most failures are one of: stale hash block, template not rebuilt after a `versioner.yaml`
edit, a depender that was not bumped, or an influential file added without updating
`influential:`.
