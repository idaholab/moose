# Where every pinned version lives

One row per thing that gets updated. "Source of truth" is the only file to edit;
"mirrors" must be kept consistent by hand (nothing generates them from the source).

Confirm against the repository rather than this table when they disagree — this is a
snapshot of the layout, and the layout does get reworked (conda per-package
`conda_build_config.yaml` files, for instance, were consolidated into
`conda/conda_build_config.yaml`).

## Submodules

Updating the submodule pointer is the whole change; no version string is written into a
recipe or def file. `scripts/apptainer_generator.py` reads the SHA straight from the
submodule, and the conda recipes build from the in-tree source.

| Submodule path | Upstream | Versioner package | Notes |
|---|---|---|---|
| `petsc` | gitlab.com/petsc/petsc | `petsc` | Track release tags (`v3.25.4`), not `main`. The `version:` in `versioner.yaml` is the bare release number. |
| `libmesh` | github.com/libMesh/libmesh | `libmesh` | Tracks upstream `devel`/master tip. Coordinate with libMesh PRs when a fix is needed. |
| `framework/contrib/wasp` | code.ornl.gov/neams-workbench/wasp (mirrored at github.com/ornl-neams-workbench/wasp) | `wasp` | |
| `framework/contrib/mfem` | github.com/mfem/mfem | `moose-dev` | apptainer only; no conda package. |
| `framework/contrib/conduit` | github.com/LLNL/conduit | `moose-dev` | Release tags (`v0.9.7`). MFEM dependency. |
| `framework/contrib/neml2` | github.com/applied-material-modeling/neml2 | `moose-dev` | API changes here usually require framework source changes in the same PR. |
| `framework/contrib/pytorch` | github.com/pytorch/pytorch | `moose-dev` | Release tags (`v2.13.0`). Drives libtorch in the container. |
| `modules/fluid_properties/contrib/saline` | github.com/ornl-neams/saline | — | Not versioner-tracked; still newsletter-worthy. |

Update with an explicit pointer move, e.g.:

```bash
git -C libmesh fetch origin && git -C libmesh checkout <sha>
git add libmesh
```

Check `git diff --submodule=short` before committing: the pointer must be a clean SHA,
never `-dirty`.

## Versions pinned in files

| What | Source of truth | Mirrors that must match | How to find the candidate |
|---|---|---|---|
| VTK | `conda/libmesh-vtk/meta.yaml.template` — `vtk_version` and `sha256` | none; `apptainer/libmesh.def` receives `vtk_url`/`vtk_sha256`/`vtk_vtk_friendly_version` from the conda recipe via `apptainer_generator.py` | vtk.org release files; recompute the tarball `sha256` |
| VTK build flags | `conda/libmesh-vtk/build.sh` | `apptainer/libmesh.def` VTK section (there is a comment in `libmesh.def` saying to keep them in sync) | — |
| Compilers, MPI, python, hdf5 and other conda-forge pins | `conda/conda_build_config.yaml` | `conda/build/conda_build_config.yaml` (slightly stricter compiler pins) | `conda-forge/conda-forge-pinning-feedstock` |
| Python tool versions (black, clang-format, clang-tools, coverage, ruff, xmltodict) | `conda/tools/conda_build_config.yaml` | `requirements.txt` (used by the apptainer containers; both files carry a comment saying they mirror each other) | conda-forge / PyPI |
| conda `moose-tools` package list | `conda/tools/meta.yaml.template` | `requirements.txt` | — |
| Mac SDK | `conda/conda_build_config.yaml` — `CONDA_BUILD_SYSROOT` | `scripts/get_mac_sdk.sh` | — |
| Base MPI container images | `apptainer/mpi.def` — `FROM_IMAGES` (digest-pinned) | the human-readable tag list in the jinja comment directly above `FROM_IMAGES` | `idaholab/moose-containers` — the images must be published there first |
| code-server, moose-language-support, python, FMI/Assimulo/PyFMI/PythonFMU, gperftools, go | `apptainer/moose-dev.def` — the `*_VERSION=` block near the top of `%post` | none | each project's releases page |
| PETSc configure options | `scripts/configure_petsc.sh` | — | often changes with a PETSc bump (e.g. dropping a `--download-*-commit` pin once upstream catches up) |
| libMesh / wasp / MFEM / conduit / NEML2 / libtorch build options | `scripts/configure_*.sh`, `scripts/update_and_rebuild_*.sh` | — | these are `influential:` files, so touching them forces a package bump |

## Which package a file belongs to

`scripts/versioner.yaml` answers this: each package's `influential:` list enumerates the
files and directories whose content feeds its hash. To check a specific file's effect:

```bash
./scripts/versioner.py <package> --yaml   # includes all_influential
```

If you add a new file that ought to affect a package's hash (a new build script, a new
file included by a def file), add it to that package's `influential:` list in the same
commit. `--verify` will show it as `NEW` under "Versioner influential files"; a `NEW`
entry you did not intend is a signal the list is wrong.

## Conda variants

`conda/conda_build_config.yaml` defines the variant matrix via `zip_keys`: `mpi` ×
`mpi_version` × `mpi_friendly_name`, plus the `python` list. The `[linux]`, `[osx]`,
`[osx and arm64]` selectors decide per-platform availability — OpenMPI is linux and
arm64-mac only. A pin that works on linux and breaks on Mac is the normal failure mode,
so read the selectors before assuming a pin is safe.
