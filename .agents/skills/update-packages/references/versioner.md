# `scripts/versioner.py` and `scripts/versioner.yaml`

## What the versioner computes

For each package in `scripts/versioner.yaml` it computes:

- `version` — read directly from `versioner.yaml`
- `build_number` — read directly from `versioner.yaml` (absent for some packages)
- `full_version` — `version` + `_` + `build_number` when a build number exists
- `hash` — a short hash over the *content* of every file in the package's `influential:`
  list, plus the hashes of its `dependencies:`

The hash is content-derived and therefore not something you set. The version *is*
something you set, and the entire verification step exists to check one invariant:

> **If a package's hash changed relative to the base ref, its `full_version` must have
> changed too.**

That is what makes a rebuilt conda package distinguishable from the one already published,
and what stops a container from being overwritten in place.

## Command reference

```bash
./scripts/versioner.py                             # moose-dev full_version at HEAD
./scripts/versioner.py <library> [commit]          # full_version of <library> at commit
./scripts/versioner.py <library> --yaml            # all computed fields, incl. influential
./scripts/versioner.py <library> --json            # same, JSON
./scripts/versioner.py --build-templates           # regenerate every templated file
./scripts/versioner.py --verify <base_ref>         # the gate; use upstream/next
./scripts/versioner.py --verify <base_ref> --brief # compact, markdown tables, no color
./scripts/versioner.py --summary                   # YAML block for versioner_hashes.yaml
```

Notes:

- The action flags (`--json`, `--yaml`, `--summary`, `--verify`, `--build-templates`) are
  mutually exclusive.
- `--verify HEAD` is rejected; pass a real base ref.
- `--brief` only works with `--verify`.
- `--summary`, `--verify` and `--build-templates` print
  `WARNING: You have changes not yet committed` when the tree is dirty. When you see it,
  the output is not trustworthy — the versioner reads committed content.
- Valid `<library>` names are the packages in `versioner.yaml`; run with `-h` to list them.

## Version-string conventions

| Style | Packages | Rule |
|---|---|---|
| Date | `tools`, `build`, `mpi`, `pyhit`, `moose-dev` | `YYYY.MM.DD` of the update itself. Must not decrease and must not be in the future. |
| Upstream release | `petsc`, `libmesh-vtk`, `seacas` | The upstream version verbatim (`3.25.4`, `9.7.0`). |
| Date + short SHA | `libmesh`, `wasp`, `pprof` | `YYYY.MM.DD_<7-char sha>`, where both come from the pinned submodule/source commit — the commit's date, not today's. |

`build_number` exists for packages that can be rebuilt without their upstream source
moving: `libmesh-vtk`, `petsc`, `libmesh`, `wasp`, `pprof`, `seacas`.

## Choosing between `version` and `build_number`

- **Upstream source moved** (new submodule SHA, new release, new VTK tarball) → bump
  `version` and **reset `build_number` to 0**. The versioner reports `BUILD NONZERO` if
  you bump a non-date version while leaving a non-zero build number.
- **Only influential files moved** (a configure flag, a conda pin, a build script) → keep
  `version`, increment `build_number`.
- **Date-versioned package, anything moved** → set `version` to today's date. These have
  no build number, so the date *is* the rebuild counter. Two updates in one month give two
  distinct dates (e.g. `2026.08.19` then `2026.08.23`).

## Fanning out to dependers

`versioner.yaml` carries a `# dependers: ...` comment above each package. The current
graph, from those comments and the `dependencies:` keys:

```
tools      -> moose-dev
build      -> moose-dev
mpi        -> petsc -> libmesh -> moose-dev
libmesh-vtk-> libmesh, moose-dev
wasp       -> pyhit, moose-dev
moose-dev  -> app
pprof, seacas: no dependers
```

Because a package's hash includes its dependencies' hashes, bumping `petsc` forces
`libmesh` and `moose-dev` to change hash, which forces them to be bumped too. Work the
graph downstream from whatever you changed, then let `--verify` confirm you got all of it.

`app` is a pseudo-package: it has templates and dependencies but no version of its own,
it is excluded from the version table, and it needs no entry in
`scripts/tests/versioner_hashes.yaml`.

## Templating

`templates:` maps `<source>.template` → `<generated file>`. The generator substitutes
`__VERSIONER_<PACKAGE>_VERSION__` and `__VERSIONER_<PACKAGE>_BUILD_NUMBER__`, where
`<PACKAGE>` is the package name uppercased with `-` → `_` (so `libmesh-vtk` becomes
`__VERSIONER_LIBMESH_VTK_VERSION__`).

**A template may only reference its own package and the packages in that package's
`dependencies:` list.** Anything else fails with:

```
Unused template variable __VERSIONER_X_VERSION__ still exists in <file>
```

The fix is to add the missing entry to `dependencies:` in `versioner.yaml` — not to
hardcode the version in the template.

Everything else in a `meta.yaml.template` (the VTK version, the `sha256`, the requirement
lists) is ordinary jinja/conda content that you edit by hand in the `.template` file.

## Interpreting `--verify`

Three tables, all compared against the base ref.

### Versioner templates

| Status | Meaning | Action |
|---|---|---|
| `OK` | Generated file matches what the template would produce | — |
| `BEHIND` | Generated file is stale | Re-run `--build-templates` and commit the result |

### Versioner influential files

Informational, never fatal. `CHANGE` / `NEW` / `REMOVED` describe how the influential set
moved relative to the base. Use it as a cross-check: a `NEW` file you did not intend to
add means the `influential:` list changed unexpectedly, and a file you *did* add that does
not appear means you forgot to list it.

### Versioner versions

| Status | Meaning | Action |
|---|---|---|
| `OK` | Hash unchanged | — |
| `CHANGE` | Hash changed and `full_version` was bumped correctly | — |
| `NEW` | Package does not exist in the base ref | Sanity-check the starting version |
| `NEED BUMP` | Hash changed, `full_version` did not | Bump `version` or `build_number` |
| `DATE DECREASE` | New date version is older than the base's | Use a date ≥ the previous one |
| `FUTURE DATE` | Date version is after today | Use today's date |
| `BUILD NONZERO` | Non-date `version` was bumped but `build_number` ≠ 0 | Reset `build_number` to 0 |

Exit code is 2 on any template or version failure, 0 otherwise. `Verification succeeded.`
plus no red is the pass condition — but note that `--brief` suppresses both the colors and
the `Verification succeeded.` line, so with `--brief` the pass condition is the exit code
plus `0 failed` in each table summary.

## `scripts/tests/versioner_hashes.yaml`

Append-only record proving the versioner still reproduces historical hashes. Format:

```yaml
<40-char commit sha>: # <PR number>
  <package>:
    full_version: <full_version>
    hash: <hash>
```

Rules:

- Generate with `./scripts/versioner.py --summary`; never hand-write the hashes.
- Append at the end, separated from the previous block by one blank line.
- Add the PR number as the trailing comment on the sha line.
- `app` is intentionally excluded.
- The sha is `git rev-parse HEAD` at the time of generation, which means **the block is
  only valid if this is the final commit**. Regenerate after any amend or rebase.
- A hash quoted as a string in the output (e.g. `hash: '2947116'`) is normal YAML
  behaviour for an all-digit value; leave it as generated.

Verified by `scripts/tests/test_versioner.py`:

```bash
cd scripts && ./run_tests --re versioner
```
