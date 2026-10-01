# Newsletter entries and PR body

The newsletter and the PR body carry nearly the same content. Write the newsletter first
(it is the version that has to survive review), then derive the PR body from it and add
the `## To do` checklist.

## Newsletter

File: `modules/doc/content/newsletter/<YYYY>/<YYYY>_<MM>.md` for the month the PR is
expected to merge. The month file already exists with empty headings; fill in the ones
relevant to the update. Read the two most recent months before writing — they are the
style authority, and the heading levels have drifted over time (`####` in mid-2026,
`###` more recently; match the file you are editing).

### `## libMesh-level Changes`

```markdown
## libMesh-level Changes

### `<new libmesh full_version>` Update

- <one bullet per user-visible libMesh change>
```

The bullets come from the libMesh commit log between the old and new submodule SHAs. This
is the longest part of the writeup and normally needs a libMesh maintainer's summary — ask
rather than inventing it from commit subjects. In the PR, track it as a
`- [ ] Update libMesh summary` to-do item.

### `## PETSc-level Changes`

One or two sentences with a compare link:

```markdown
The PETSc submodule was updated from [3.25.2 to 3.25.4](https://gitlab.com/petsc/petsc/-/compare/v3.25.2...v3.25.4).
```

### `## Conda Package Updates`

One `###` block per bumped package, in `versioner.yaml` order (dependency order), titled
with the package's new full version exactly as `./scripts/versioner.py <pkg>` reports it,
plus the variant list where the package has one:

```markdown
### `moose-libmesh-vtk 9.7.0_1 [mpich,openmpi]`

- Trigger rebuild; no package changes

### `moose-petsc 3.25.4_0 [mpich,openmpi]`

- Updated petsc from [3.25.2 to 3.25.4](https://gitlab.com/petsc/petsc/-/compare/v3.25.2...v3.25.4)

### `moose-libmesh 2026.08.18_0772c3d_0 [mpich,openmpi]`

- Updated libmesh from [`ab36c00` to `0772c3d`](https://github.com/libMesh/libmesh/compare/ab36c007ae608290e097bdfbc2766a46d8192b70...0772c3d45eac8019a76f6dcdb8683231fe354c57)

### `moose-pyhit 2026.08.19`

- Rebuild due to updated Conda package `moose-wasp 2026.08.13_ce25dcd`
```

Naming and phrasing conventions:

- conda package names are the `moose-` prefixed recipe names (`moose-tools`,
  `moose-libmesh-vtk`, `moose-dev`), not the versioner package names.
- variant suffix `[mpich,openmpi]` for MPI-variant packages; omit for `moose-tools`,
  `moose-build`, `moose-pyhit`, `moose-wasp`, `moose-pprof`, `moose-seacas`.
- `Trigger rebuild; no package changes` — bumped only because a pin or an influential file
  moved, with no substantive change.
- `Rebuild due to updated pins in \`conda/conda_build_config.yaml\`` — bumped because of
  the conda-forge pin sweep.
- `Rebuild due to updated <dep> dependency \`<name> <full_version>\`` or
  `Rebuild due to updated dependencies` — bumped because something upstream in the graph
  moved.
- Submodule/source moves use a compare link: full 40-char SHAs in the URL, 7-char SHAs in
  the link text.
- Every package that got a version bump needs a block, even a pure-rebuild one. The set of
  blocks should be exactly the set of `CHANGE` rows from `--verify`.

A second update in the same calendar month **appends a new set of blocks** after the first;
do not edit the earlier blocks.

### `## Apptainer Package Updates`

Same idea, keyed by container URI (`<name>:<tag>`) rather than package name:

```markdown
### `moose-mpi:2026.08.23`

- Update all base containers
- Build valgrind with `--enable-lto=yes`

### `moose-petsc:3.25.4_1`

- Add `OPTFLAGS` generator variable to override `OPTFLAGS`
```

For a base-image sweep, enumerate the variants with their toolchain versions, taken from
the human-readable tag comment above `FROM_IMAGES` in `apptainer/mpi.def`:

```markdown
- Updated variants:
  - `ubuntu24_gcc` (default): Ubuntu 24.04 base with GCC 14.2.0, MPICH 5.0.1, OpenMPI 5.0.10
  - `ubuntu24_gccmin`: Ubuntu 24.04 base with GCC 9.5.0, MPICH 5.0.1
```

Call out anything that removes platform support explicitly — past entries note when a CUDA
bump dropped support for specific INL GPU hardware. That is the detail users actually need
from this section.

### Other newsletter sections

Source fixes that rode along with the update (API adaptations for a new NEML2, test
regolds, compiler-warning fixes) belong under `## MOOSE Improvements`,
`## MOOSE Bug Fixes`, or `## MOOSE Modules Changes`, with the usual
`([idaholab/moose#NNNNN](https://github.com/idaholab/moose/pull/NNNNN))` attribution.

## PR body

Not the standard `.github/PULL_REQUEST_TEMPLATE.md`. `AGENTS.md` section 9 says not to
impose boilerplate headings on a PR description; a package update is the case that earns
them, because the body is a per-package changelog that reviewers diff against the
newsletter. Use the headings below and nothing else — in particular, no "Summary",
"Test plan", "Verified", or "Future work" sections.

Layout, in order:

1. A `## <Library> update [\`old..new\`](compare-url)` section for each major source bump,
   with the same bullets used in the newsletter's library sections.
2. `## Conda packages` — the same blocks as the newsletter.
3. `## Apptainer containers` — the same blocks as the newsletter.
4. `## To do` — a live checklist.

The `## To do` list is the part the PR body adds over the newsletter, and it is what makes
the PR reviewable while still in flight. Typical items:

```markdown
## To do

- [ ] Update libMesh summary
- [ ] Update newsletter documentation
- [ ] Wait for PETSc 3.25.2
- [ ] Use production images from moose-containers
- [ ] Remove `--download-umpire-commit=v2025.12.0` from `configure_petsc.sh`
- [ ] MFEM mac conda build
- [ ] Fix libmesh intel build (requires https://github.com/libMesh/libmesh/pull/4474)
- [ ] Enable conda "optional" recipes upon merge
```

Keep it checked off as work lands; reviewers read it as the merge gate. Items that
reference another repository's PR or a pending upstream release are the reason a package
update PR stays open for weeks — make those explicit rather than leaving them in commit
messages.

## PR mechanics

- Base: `next`.
- Title: `Update packages`, or `Update packages, <the notable extras>` when the PR also
  reworks something (`Update packages, add ubuntu containers, add compiler optimization`).
- Label: `PR: Updates packages`.

```bash
gh pr edit <n> --add-label "PR: Updates packages"
```

- Once the PR number exists, add it as the trailing comment on the
  `scripts/tests/versioner_hashes.yaml` entry and amend the final commit.

## Commit-history shape

Package-update PRs end with these three commits, in this order:

```
Update packages                      # versioner.yaml bumps + regenerated templates
Update newsletter with package changes
Update versioner hashes
```

Everything else — submodule bumps, pin sweeps, def-file changes, source fixes, regolds —
comes before them, one logical change per commit. Past PRs have run to 60+ commits; that is
normal and reviewers rely on the granularity.
