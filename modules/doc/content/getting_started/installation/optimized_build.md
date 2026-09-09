# Building the MOOSE stack with native optimizations

This guide builds **PETSc**, **libMesh**, and **MOOSE** from source, tuned to the build
host (`-march=native`) and aggressively optimized (`-Ofast`), **without modifying any of
MOOSE's build scripts**. The compiler flags are passed as arguments at invocation; the
`scripts/update_and_rebuild_*.sh` scripts forward unrecognized arguments straight to
`configure`, so no local diffs against upstream are required.

> Compiler flags only take effect at compile time, so applying them means rebuilding each
> component from source. There is no way to re-flag an already-compiled library.

## Read this first: the `-Ofast` / fast-math caveat

`-Ofast` enables `-ffast-math` (which includes `-ffinite-math-only`). Consequences to be
aware of before you commit to it:

- The compiler is told to assume `NaN`/`Inf` never occur, so `isnan`/`isinf` checks can be
  optimized away. PETSc and MOOSE use these for solver divergence/convergence detection and
  `-fp_trap`; robustness of *failing* solves may be affected (well-conditioned solves are
  fine).
- It overrides libMesh's deliberate `-ftrapping-math` and produces
  `-Wnan-infinity-disabled` warnings in Eigen's `SelfAdjointEigenSolver` — i.e. in MOOSE's
  `RankTwoTensor` eigen-decomposition, used heavily in solid mechanics.
- On Clang, `-Ofast` is deprecated and prints a warning (it still works); Clang maps it to
  `-O3 -ffast-math`.

If that trade-off is not wanted, substitute one of these everywhere `-march=native -Ofast`
appears below:

- `-O3 -march=native -ffast-math` — identical codegen to `-Ofast`, no deprecation warning.
- `-O3 -march=native` — keeps native tuning, drops fast-math (safest numerically).

## Prerequisites

- **A compiler/MPI-only conda environment** (a `moose-mpi` style env, *not* the prebuilt
  `moose-dev` package, since we are building PETSc/libMesh ourselves).

- **Submodules checked out and in sync with the MOOSE revision.** Passing
  `--skip-submodule-update` leaves them stale, so sync them up front:

  ```bash
  git submodule update --init --recursive
  ```

- **WASP** is also required by MOOSE. Build it normally with
  `scripts/update_and_rebuild_wasp.sh`; it is not performance-critical (input parsing only)
  and is out of scope for this guide.

- Set a parallel job count once and reuse it:

  ```bash
  export MOOSE_JOBS=$(getconf _NPROCESSORS_ONLN)   # or a smaller number if not a lot of RAM
  ```

All commands below are run from the MOOSE root directory.

## 1. PETSc

```bash
./scripts/update_and_rebuild_petsc.sh \
    COPTFLAGS="-march=native -Ofast" \
    CXXOPTFLAGS="-march=native -Ofast" \
    FOPTFLAGS="-march=native -Ofast"
```

`COPTFLAGS` / `CXXOPTFLAGS` / `FOPTFLAGS` are PETSc's dedicated *optimization*-flag
variables. Use these rather than `CFLAGS`/`CXXFLAGS`/`FFLAGS`:

- they are the flags PETSc uses when `--with-debugging=no`,
- they are propagated to the packages PETSc downloads and builds (hypre, MUMPS,
  SuperLU_DIST, STRUMPACK, Kokkos, OpenBLAS, SLEPc, ...), and
- unlike `CFLAGS`/`FFLAGS`, they do not trigger PETSc's "you are overwriting the standard
  flags" warning.

The result installs into `petsc/arch-moose`.

**Verify:**

```bash
make -C petsc PETSC_DIR=$PWD/petsc PETSC_ARCH=arch-moose check
grep -E '^(CC|CXX|FC)_FLAGS' petsc/arch-moose/lib/petsc/conf/petscvariables   # should show -march=native -Ofast
```

## 2. libMesh

```bash
METHODS=opt PETSC_DIR=$PWD/petsc PETSC_ARCH=arch-moose \
  ./scripts/update_and_rebuild_libmesh.sh \
    libmesh_CFLAGS="-march=native -Ofast" \
    libmesh_CXXFLAGS="-march=native -Ofast"
```

Key points:

- **Use `libmesh_CXXFLAGS` / `libmesh_CFLAGS`, not plain `CXXFLAGS` / `CFLAGS`.** libMesh
  folds `libmesh_CXXFLAGS` into each method's `CXXFLAGS_OPT`, so the flags are used both to
  build libMesh itself *and* are exported by `libmesh-config` — which is what MOOSE compiles
  against. Plain `CXXFLAGS` would only reach libMesh's own objects and would never make it
  into the flags MOOSE sees.
- **`METHODS=opt`** builds only the optimized method (a `-Ofast` "debug" method is
  contradictory and multiplies build time). Build MOOSE with `METHOD=opt` to match.
- The result installs into `libmesh/installed`.

**Verify:**

```bash
METHOD=opt libmesh/installed/bin/libmesh-config --cxxflags   # should contain -march=native -Ofast
```

## 3. MOOSE

MOOSE reads libMesh's exported flags via `libmesh-config`, so once libMesh is built as
above, MOOSE's own sources compile with `-march=native -Ofast` automatically — no extra
flags on the MOOSE build:

```bash
cd test        # framework test app; or your application directory
METHOD=opt PETSC_DIR=$MOOSE_DIR/petsc PETSC_ARCH=arch-moose \
  LIBMESH_DIR=$MOOSE_DIR/libmesh/installed \
  make -j "$MOOSE_JOBS"
```

`METHOD=opt` matches the opt-only libMesh built above. (`MOOSE_DIR` is the MOOSE root; the
defaults for `LIBMESH_DIR` and the WASP location already point at the in-tree
`libmesh/installed` and `framework/contrib/wasp/install`, so setting `LIBMESH_DIR` here is
just to be explicit.)

**Verify:**

```bash
# whole optimized stack loads at runtime
./moose_test-opt --version

# run the test suite: some tests will fail due to -Ofast / -ffast-math
cd tests
./run_tests -j 10
```

You can confirm MOOSE's own objects are being compiled with the flags with a dry run:

```bash
make -n | grep -m1 -oE 'mpicxx .*-c .*\.C' | tr ' ' '\n' | grep -E 'march=native|Ofast'
```

## Quick reference

| Component | How the flags get in | Installs to |
|-----------|----------------------|-------------|
| PETSc | `COPTFLAGS` / `CXXOPTFLAGS` / `FOPTFLAGS` args → `configure` (and downloaded packages) | `petsc/arch-moose` |
| libMesh | `libmesh_CFLAGS` / `libmesh_CXXFLAGS` args → `CXXFLAGS_OPT` (own build + `libmesh-config` export) | `libmesh/installed` |
| MOOSE | inherited automatically from `libmesh-config` | your app dir (`*-opt`) |
