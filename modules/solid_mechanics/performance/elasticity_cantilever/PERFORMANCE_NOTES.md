# libMesh/MFEM parity notes: the elasticity cantilever

This file is the running record of work to close the performance gap between the MOOSE libMesh
backend and the MOOSE MFEM backend on one benchmark, the linear elasticity cantilever in this
directory (`kokkos_cantilever.i` and `mfem_cantilever.i`). It exists because conclusions about this
benchmark have been reversed more than once, in both directions, by people acting on numbers that
turned out to have been measured against a stale binary or an unmatched configuration.

Background discussion: MOOSE discussion #33740.

This file is deliberately not tracked. It is listed in the repository's `.git/info/exclude`, so it
stays out of commits while remaining where the benchmark it describes lives.

## Start here

**The conclusion, as of 2026-09-22: there is not much left to win on this benchmark.** At 24 ranks and
refinement 5 libMesh runs it in 6.03 s against MFEM's 5.00 s, a factor of 1.21, down from 1.43 at the
start of that day. The linear solve is already *faster* than MFEM's. Of the remaining 1.03 s, about
0.54 s is structural work in the assembly path, of which the largest single item is irreducible on our
side, and about 0.45 s is framework generality that MFEM has no counterpart for and that could only be
closed by removing capability. A session that wants a large number from this benchmark should change
the benchmark, not the code.

**Always state the rank count with any ratio.** It is 1.44x at 6 ranks and 1.25x at 24, because libMesh
scales better here (71% parallel efficiency against 62% from 6 to 24 ranks). Nothing below 6 ranks has
been measured on the MFEM side, and that is where a workstation user sits.

**Three things worth reading before anything else.** The editing rules immediately below, which exist
because each was violated at a cost. The refuted table, which is the most valuable part of this file:
roughly twenty plausible ideas that were tried or reasoned through and did not work, several of them
recommendations made and then withdrawn within the same session. And the note at the end of finding 21
about a single mistake made three separate times, mistaking a decomposition for a causal model, which
is the failure this file is least able to protect you from on its own.

**How to read the findings.** They are numbered in discovery order, not importance, and several correct
earlier ones. Read a correction before the thing it corrects: finding 8 is withdrawn entirely and
replaced by 14, which also corrects the per-iteration figures in findings 1 and 7; finding 9 was
rewritten after its recommendation was withdrawn; finding 21 corrects the attribution in finding 20.

**On the numbers.** Run-to-run and machine-load variation is around 0.1 s at 24 ranks, so treat
differences below roughly 0.15 s as needing repeat runs. Figures written with a leading `~` are derived
by applying instruction shares from a serial profile to measured parallel wall times, not measured
directly; they are good to about 20% and are marked wherever they appear.

## Rules for editing this file

Follow these even when they feel like overhead. Every one of them exists because it was violated
and cost a session.

1. **No number without its build identity.** Record the MOOSE commit, the libMesh install prefix,
   and the `Executable Timestamp` from the MOOSE console header. A figure whose provenance cannot
   be reconstructed is not evidence and should be deleted rather than repeated.
2. **State the rank count, thread count, and AMG configuration of both sides of any comparison.**
   Mismatches in these three produced at least four wrong conclusions across the sessions behind
   this file.
3. **Refuted claims stay in the file.** Move them to "Refuted" with the measurement that killed
   them. Deleting them invites the next person to rediscover and re-refute them. If you refute
   something listed as established, move it rather than editing it away, and say what you measured.
4. **Distinguish measured from inferred.** A mechanism that was reasoned about but never isolated
   is a hypothesis; label it so. Two items in the history of this benchmark were carried for
   sessions as established fact while never having been measured, and both turned out to be wrong.
5. **Rebuild before measuring.** See "Build traps" below; two separate stale-binary incidents are
   in the history.

## Standing measurement

Taken 2026-09-22 on the committed state of branch `kokkos-pa` (through "Match MFEM's relaxation
ordering in the cantilever comparison"), libMesh from `libmesh/installed`, PETSc 3.25.4, MFEM 4.9.1.
24 MPI ranks, one thread per rank, `refinement=5`, 262,144 HEX8 elements, 839,619 degrees of freedom,
file output off. Both backends are run from the same executable. libMesh uses the fast configuration:
`DistributedRectilinearMeshGenerator` with `Mesh/parallel_type=DISTRIBUTED`, so the mesh is generated
already distributed and never refined. Wall times are the median of three runs.

| quantity                     | libMesh | MFEM  |
| ---------------------------- | ------- | ----- |
| wall (s)                     | 6.03    | 5.00  |
| CPU-seconds                  | 138.4   | 113.7 |
| ratio                        | 1.21x   | -     |
| deficit                      | 1.03 s  | -     |

The libMesh side of this table was taken before the cantilever moved onto the production solid
mechanics objects. Finding 22 re-measures it after that migration and finds it unchanged, so the
table still stands.

Decomposed. Wall time minus the mesh and minus the linear solve is where the entire deficit now lies.

| phase                                      | libMesh | MFEM  | deficit |
| ------------------------------------------ | ------- | ----- | ------- |
| mesh construction                          | ~0.67   | 0.68  | -0.01   |
| linear solve                               | 2.93    | 3.08  | **-0.14** |
| weak form to assembled system              | ~1.33   | 0.78  | **+0.54** |
| everything else, including process startup | ~1.10   | 0.46  | **+0.64** |
| total                                      | 6.03    | 5.00  | 1.03    |

Every number here is measured on the current build; none is inherited. The rows sum to the measured
deficit.

The linear solve is faster than MFEM's, by 0.14 s: `KSPSolve` 2.618 plus `PCSetUp` 0.315 against
MFEM's instrumented solve section at 3.077 s, which agrees with hypre's own `PCG Setup` plus
`PCG Solve` to three decimal places. 84 iterations on both sides, so per iteration 0.0312 s against
0.0333 s, about 6% ahead.

"Weak form to assembled system" is the like-for-like comparison, because MFEM does its sparsity work
inside assembly rather than in a separate pass. MFEM's side is one measured number, the `FormSystem`
section at 0.783 s, covering element matrices, the sparsity it builds while assembling them,
essential boundary condition elimination and parallel assembly of the operator. The libMesh side is
the sum of element kernels 0.834, `DofMap::compute_sparsity` ~0.16, `DofSpace::setupSparsity` ~0.03,
`MatSetPreallocationCOO` 0.262, `MatSetValuesCOO` 0.037 and libMesh's matrix preallocation ~0.004.
Only 0.05 s of the 0.54 s deficit is the element kernel; the rest is structure, spread across the
three representations of the graph described in finding 18.

MFEM's `FormSystem` plus its solve section account for the whole of its executioner, 3.86 s, so MFEM
has essentially no other execute work at all. Its 0.46 s of "everything else" is process startup and
teardown outside both phases. Ours is 0.17 s of startup plus roughly 0.9 s of setup and execute work,
now itemized in finding 19: it is a long tail rather than a single item.

Run-to-run and machine-load variation on this benchmark is around 0.1 s at 24 ranks, and larger when
the machine is busy. Treat differences below about 0.15 s as needing repeat runs.

### Gains made on 2026-09-22

Start of day, on the first consistent build of the session, 7.14 s at 1.43x. End of day 6.03 s at
1.21x: **1.11 s, or 15%**. Three changes contributed, and it matters which is which.

| change | wall | what moved |
| ------ | ---- | ---------- |
| `-nl0_mat_type hypre` | 7.14 -> 6.97 | `MatConvert` 0.104 s eliminated; `MatMult` 0.466 -> 0.269 |
| single-component gradient table | 6.97 -> 6.81 | element kernel 1.139 -> 0.834 |
| `-pc_hypre_boomeramg_no_CF` | 6.81 -> 6.18 | `PCApply` 3.006 -> 2.315 at unchanged iterations |

Read honestly, only the middle one makes MOOSE faster for anyone who does not edit their input. The
other two are corrections to how this benchmark configured hypre relative to MFEM, worth 0.80 s of the
1.11 s, and a user would have to opt into both. The framework change is worth about 0.3 s on the
element kernel and applies to any Kokkos vector kernel, not only to this benchmark.

A fourth change, allocating the Kokkos matrix values after the COO preallocation, is memory hygiene
with no measurable effect on wall time.

The correctness anchor throughout is the tip deflection, `maxZ = -2.924858e-10` at
`refinement = 5`. It has not changed across any configuration or code change recorded here. The
committed test spec checks it at the cheap default `refinement = 3`.

## Reproducing

Environment is the apptainer container; there is no conda in this stack. `PETSC_DIR` points into
the read-only `/opt` tree, which is correct; libMesh comes from the submodule installed into
`libmesh/installed`. `METHOD` is `dbg` in the environment while every binary used here is `opt`,
which bites both `make` and `run_tests`.

```
# build
LIBMESH_DIR=$PWD/libmesh/installed METHOD=opt make -j64 -C modules/solid_mechanics

# libMesh, fast configuration.  -log_view must come LAST: placed before the Mesh/ arguments it
# consumes the next token as a viewer name and aborts with "Unsupported viewer Mesh/gen_mesh/...".
D=modules/solid_mechanics/performance/elasticity_cantilever
mpiexec -n 24 modules/solid_mechanics/solid_mechanics-opt -i $D/kokkos_cantilever.i \
  refinement=5 --n-threads=1 --timing \
  Mesh/gen_mesh/type=DistributedRectilinearMeshGenerator Mesh/parallel_type=DISTRIBUTED \
  -log_view

# MFEM; hypre prints its own setup and solve timers at print_level = 1
mpiexec -n 24 modules/solid_mechanics/solid_mechanics-opt -i $D/mfem_cantilever.i \
  refinement=5 --timing

# tests, from modules/solid_mechanics
METHOD=opt ./run_tests -i performance -j 12

# the Kokkos suite, from test.  -i renames the spec file rather than filtering, so "-i kokkos"
# finds nothing; --re filters by test name.
METHOD=opt ./run_tests --re kokkos -j 12
```

The framework change behind finding 4a was verified against 215 passing Kokkos tests and 54 passing
vector-FE tests, with the three `elasticity_cantilever` performance tests.

### Profiling

There is no `perf` in the container and `perf_event_paranoid` is 2, so sampling is unavailable.
`valgrind` 3.27.1 and `gprof` are present. `METHOD=oprof` compiles at `-O2 -g`, which is what
callgrind needs for symbols; `libmesh_oprof` is installed alongside the other methods.

```
# oprof build
LIBMESH_DIR=$PWD/libmesh/installed METHOD=oprof make -j56 -C modules/solid_mechanics

# instruction profile of the solve, serial, collection toggled on at SNESSolve so the mesh and
# setup phases stay out of it
OMP_NUM_THREADS=1 /opt/valgrind/bin/valgrind --tool=callgrind \
  --collect-atstart=no --toggle-collect=SNESSolve --cache-sim=yes \
  --callgrind-out-file=cg.out \
  modules/solid_mechanics/solid_mechanics-oprof -i $D/kokkos_cantilever.i refinement=4 \
  --n-threads=1 Mesh/gen_mesh/type=DistributedRectilinearMeshGenerator \
  Mesh/parallel_type=DISTRIBUTED
/opt/valgrind/bin/callgrind_annotate --inclusive=yes cg.out
```

Callgrind counts instructions and simulated cache traffic, not kernel time, so a cost that is
absent from its profile is evidence in itself; finding 8 rests on exactly that. Run single rank when
differencing `-log_view` events, because the times PETSc reports are maxima over ranks and do not
difference correctly otherwise.

PETSc's own source is available in the `petsc` submodule, pinned at the version that is installed
under `/opt/petsc`, so its behaviour can be read rather than guessed. Finding 13 rests on that, and
two wrong conclusions in this file came from reasoning about PETSc from its API alone.

#### GPU profiling, and the flag without which it is useless

**Configure libMesh `--with-nvtx` before profiling anything on a GPU.** `PerfGuard` already pushes an
NVTX range named after the perf graph section it is timing, so every `TIME_SECTION` in MOOSE becomes
a labelled range in an Nsight Systems timeline at no cost in source changes. It is compiled out
unless `LIBMESH_HAVE_NVTX_API` is defined, which comes from libMesh's `--with-nvtx`, and
`libmesh_config.h` carries `#undef HAVE_NVTX_API` by default. Without the flag a GPU trace contains
no ranges at all and nothing in it can be attributed to a phase of the solve.

```
METHODS=opt ./scripts/update_and_rebuild_libmesh.sh --skip-submodule-update   --with-nvtx=/usr/local/cuda
```

Confirm with `--show-capabilities`, which reports `nvtx_api` once it is on. The capability is
registered against the flag name, so `Capabilities.C` is where to look if the spelling changes.

This is not a refinement. Without ranges, a trace of the NEML2 constitutive update looked
launch-latency-bound, and the conclusion drawn from it was wrong: the cost was half a million
eighty-byte device-to-host transfers in a bridge that the trace could not attribute, and the same
trace with ranges placed 207,360 of them in the assembly phase against 727 in the model solve. A
GPU trace without NVTX ranges is not weak evidence, it is misleading evidence.

`nsys` and `ncu` ship inside the Nsight Compute tree rather than on the path, at
`/opt/nvidia/nsight-compute/<version>/host/target-linux-x64/`. The bundled Python there is stripped
and lacks `_posixshmem`, so any tooling that runs report SQL through it fails with a
`ModuleNotFoundError` while reporting its dependencies as healthy; putting the system
`/usr/lib/python3.12/lib-dynload` on `PYTHONPATH` supplies the missing extension without touching the
vendored install.

#### The CUDA container is a different environment

The measurements in this file were taken in the MPICH container. The CUDA container
(`moose-dev-ubuntu24-cuda-gcc-openmpi`) is OpenMPI, so libMesh must be rebuilt rather than reused,
and every generated `.la` in the tree names the old MPI and has to be deleted or the final link fails
looking for `/opt/mpich/lib/libmpicxx.la`. `make clean` from an application directory does not reach
sibling modules, so the archives have to be found and removed directly.

MFEM cannot currently be built there at all, so the comparison this file is about cannot be run in
it. `update_and_rebuild_mfem.sh` hits two separate failures against a CUDA PETSc: SuperLU_DIST's
GPU-enabled headers need `$CUDA_DIR/include` on the compile line, which the script never adds and
which surfaces as the misleading `*** SuperLUDist not found`; and MFEM refuses to build against a
CUDA-enabled hypre without `MFEM_USE_CUDA=YES`, which the script never sets.

On the libMesh side `-log_view` gives `PCSetUp` and `KSPSolve`, and
`-pc_hypre_boomeramg_print_statistics` prints the hierarchy so the two can be compared level by
level. On the MFEM side hypre's own `PCG Setup` and `PCG Solve` timers do the same job.

### Build traps

Both of these have already produced wrong conclusions once each.

- **Check `Executable Timestamp` in the MOOSE console header against your tree before trusting a
  number.** The first handoff reported figures from a binary built from a tree state matching no
  commit, running against a libMesh shared library reinstalled 23 seconds earlier.
- **Stale libtool archives silently redirect the link to a different libMesh.** Generated `.la`
  files record the `LIBMESH_DIR` they were built against in `dependency_libs`, and libtool puts
  that path ahead of the current `LIBMESH_DIR` on every downstream link. In September 2026 the
  archives under `framework/contrib/` still named a previous `libmesh/installed-devel` prefix,
  so the build compiled against `libmesh/installed/include` and linked against the older library,
  which surfaced as undefined references to `MeshBase::spatial_dimension`,
  `PetscMatrixShellMatrix<double>` members and `Elem::face_orientation`. The fix is to delete the
  stale archives and relink. Confirm the result with
  `readelf -d modules/solid_mechanics/solid_mechanics-opt | grep RUNPATH` and
  `ldd ... | grep libmesh_`, both of which must name the prefix you intended. Had the link
  succeeded instead of failing, the binary would have run against the wrong libMesh silently.

## Established findings

Each entry says what was measured, not what was reasoned.

### 1. Assembling the Jacobian directly as a hypre matrix is worth about 0.17 s (2026-09-21)

`-nl0_mat_type hypre` requires no code change. MOOSE's `applyMatrixTypeOptions`
(`framework/src/utils/PetscSupport.C:248`, called from `petscSetDefaults`) applies the prefixed
matrix-type option before the first assembly, libMesh's `PetscMatrix` already carries a `HYPRE`
case in `PetscMatrixType` (`libmesh/include/numerics/petsc_matrix.h:47`), and the Kokkos assembly
already fills the matrix through PETSc's COO interface (`MatSetPreallocationCOO` and
`MatSetValuesCOO` in `framework/src/kokkos/systems/KokkosMatrix.K`), which `MATHYPRE` implements in
PETSc 3.25.4. `maxZ` and the matvec count are unchanged, so the operator is identical.

Composition of the win, means of three runs each:

| `-log_view` event                    | AIJ   | MATHYPRE | delta  |
| ------------------------------------ | ----- | -------- | ------ |
| `MatConvert`                         | 0.104 | absent   | -0.104 |
| `MatMult` (85)                       | 0.466 | 0.269    | -0.197 |
| `PCSetUp`                            | 0.412 | 0.320    | -0.092 |
| `PCApply` (85)                       | 2.828 | 2.929    | +0.102 |
| `KokkosSystem::MatSetPreallocation`  | 0.197 | 0.258    | +0.061 |

The same combination is already exercised, on GPU only, by
`test/tests/kokkos/petsc_gpu/kokkos_2d_diffusion_tag_vector.i`, which sets
`-vec_type kokkos -nl0_mat_type hypre`.

### 2. PETSc's MPIAIJ matrix-vector product is 1.7x slower than hypre's ParCSR one here (2026-09-21)

`MatMult` over 85 applications: 0.466 s on MPIAIJ against 0.269 s on a natively built ParCSR, with
markedly lower run-to-run variance in the second. This is the whole of the per-iteration difference
that finding 1 recovers.

### 3. The remaining gap is dominated by assembly, not by the solver (2026-09-21)

From the decomposition above: assembly and other execute 1.76 against 0.79, a deficit of 0.97 s,
against 0.44 s in setup and 0.51 s in the solve. Within the solve only 0.17 s is inside the linear
solver at all.

### 4. A vector element kernel is limited by shape table traffic, not by multiply count (2026-09-21)

`mfem::ElasticityIntegrator` exploits the fact that a vector Lagrange shape function has one
nonzero component. `Moose::Kokkos::VectorKernelGrad::computeJacobianInternal` instead evaluates
`value.contract(_grad_test.reference(datum, i, qp))`, a dense 3x3 double contraction, for every
one of 24 x 24 x 8 `(i, j, qp)` triples, so six of every nine multiplies land on structural zeros.

Attributed by substitution, not inference: replacing the kernel with `KokkosVectorDiffusion`, which
returns `_grad_phi(j, qp)` and does no constitutive work through the identical hook, still cost
0.625 s of the 0.962 s element kernel. Substituting literal Lame parameters for the material
properties gave 0.913 s. So the framework loop is about 65% of the kernel, `stress()` about 30%,
and material property lookups about 5%.

The sparsity is a property of the stored table, confirmed by reading: `referenceGradient` in
`framework/src/kokkos/base/KokkosAssembly.K` documents and fills row `a`, column `b` of the
`Real33` as the derivative of component `a` with respect to reference direction `b`, so a
single-component shape function leaves exactly one nonzero row, holding the reference gradient of
the underlying scalar basis function. The contraction therefore reduces to a 3-term dot product
against that row.

Which part of that costs time was established by trying both halves separately, and the answer is
not the one the arithmetic suggests. Cutting the multiply count from nine to three while still
reading the dense `Real33` made assembly *slower*; storing the nonzero row on its own so the loop
reads three entries instead of nine made it 27% faster. The loop is limited by how much of the
shape table it reads and by load latency, so an optimization that reduces multiplies without
reducing loads is a regression. See the refuted table.

The obvious shortcut, treating `i % dim` as the component, is a `LAGRANGE_VEC` ordering assumption
rather than a guarantee, and must not be used. `build_tensor_tables` in the same file is the
precedent to follow: it writes down a basis-specific factorization, validates it numerically
against the dense table it claims to reproduce, and leaves consumers on the dense table when
validation fails.

### 4a. The single-component gradient table is worth 0.305 s of assembly (2026-09-21)

`KokkosAssembly` now condenses the reference vector shape gradient into a pool of

```
struct ComponentGradient { Real3 gradient; unsigned int component; };
```

one entry per pool row and quadrature point, so a consumer reads 32 bytes rather than a 72-byte
`Real33` and gets the component on the same cache line instead of through a second indirection.
The three `VectorKernelGrad` loops contract against it through `Real33::contractRow`.

The structure is validated against the dense table it condenses, entry by entry against exact zero,
and a family whose shape functions carry more than one component leaves the pool empty and its
consumers on the full contraction. The dense table is retained, so `_grad_phi`, the curl path and
`KokkosFESystem` are untouched and the AD paths never see this.

`KokkosKernel` fell from 1.139 s to 0.834 s and wall time from 6.97 s to 6.77 s, with `maxZ`
unchanged and the same 85 matvecs. Assembly is now roughly at parity with MFEM's 0.74 s.

Note for whoever extends this: `ComponentGradient` must be declared outside the
`#ifdef MOOSE_KOKKOS_SCOPE` guard in `KokkosAssembly.h`, beside `TensorMode`, because
`KokkosAssembly`'s member declarations are compiled by framework translation units that do not
define that macro.

The fallback branch is not covered by any test. `LAGRANGE_VEC` is the only vector family appearing
in the Kokkos test suite, so no test exercises a vector family for which validation fails. The
validation logic is what protects that path, and it has not been observed to fire.

### 5. The Kokkos assembly owns the matrix values outright (2026-09-21)

Dirichlet rows are eliminated in the device-side value array through
`Moose::Kokkos::Matrix::zero(row)` (`framework/include/kokkos/systems/KokkosMatrix.h:69`) before
the COO submission; `MatZeroRows` is never called on this path. The only PETSc matrix operations in
a run are `MatGetCurrentMemType`, `MatSetPreallocationCOO`, `MatSetValuesCOO`, `MatZeroEntries`,
and what `KSPCG` and `PCHYPRE` need.

This matters for finding 6: symmetric elimination could be done in the Kokkos value array, folding
the constraint contribution into the right-hand side, without needing `MatZeroRowsColumns` and
without touching the traditional `Assembly.C` path.

### 6. Essential constraints are eliminated asymmetrically (measured 2026-09-20, not yet re-measured)

MFEM eliminates rows and columns symmetrically, folding the constraint contribution into the
right-hand side. MOOSE zeroes the row and the nodal boundary condition then contributes a diagonal
of 1.0, leaving the columns in place; there is no `MatZeroRowsColumns` path anywhere in MOOSE.

Evidence is hypre's level-0 row sums, from `-pc_hypre_boomeramg_print_statistics` and MFEM's
`print_level = 1`: libMesh spans `[-8.285e-06, 1.000e+00]` and MFEM `[-5.406e+08, 3.509e+09]`.
libMesh's interior rows still sum to approximately zero, which is equilibrium with rigid
translation in the null space, because the boundary columns are still present; its constrained rows
sum to exactly 1.0 because the row is a unit row.

Consequences: the two operators are genuinely different, so unequal iteration counts are expected
rather than contradictory; `-ksp_type cg` is being applied to a matrix that is not symmetric; and
the diagonal spans 1.0 on constrained rows against roughly 1e11 on interior rows, since
`lambda = 60.5e9`. How much this costs has never been isolated. Testing `automatic_scaling` as a
remedy is blocked by the bug below.

### 7. Non-solver, non-kernel execute work is 0.94 s and is now the largest deficit (2026-09-21)

Broken down from the `PerfGraph` tree of the fast configuration, timing-root rank. `execute` is
`MooseApp::executeExecutioner` at 5.056 s.

| item                                                    | s     | where |
| ------------------------------------------------------- | ----- | ----- |
| inside `FEProblem::solve` but outside `KSPSolve`         | 0.332 | SNES and MOOSE level work around the linear solve |
| `KokkosSystem::MatSetPreallocation`                      | 0.256 | first Jacobian evaluation, via `KokkosCopy` |
| `NonlinearSystemBase::KokkosReinit` (4 calls)            | 0.097 | residual and Jacobian paths, `system.reinit()` |
| `FEProblem::projectSolution`                             | 0.059 | `initialSetup` |
| `FEProblem::computeUserObjects`                          | 0.056 | `PicardSolve` |
| `KokkosSystem::MatSetValues`                             | 0.047 | `KokkosClose`, the COO fill |
| `Console::outputStep`                                    | 0.026 | |
| remainder, in self times of the enclosing sections       | ~0.07 | |

The call path for the preallocation, which is the largest item with an identifiable cause, is
`NonlinearSystemBase::computeKokkosJacobian` -> `KokkosCopy` ->
`Moose::Kokkos::System::sync(HOST_TO_DEVICE)` -> `Matrix::create` -> `MatSetPreallocationCOO`. It is
paid once per run, in execute rather than setup.

This is a different target from the refuted item about constructing the matrix as `MATHYPRE` from
the start, which concerned libMesh's redundant AIJ preallocation. The question here is the cost of
the COO path itself: `DofSpace::setupSparsity` already holds a CSR derived from libMesh's sparsity
pattern, and handing PETSc an unsorted triplet list makes it re-derive structure that is already
available in sorted form.

An earlier version of this file put this work at 0.6 s. That was arithmetic error: it subtracted
`FEProblem::solve` self time, which already contains `KSPSolve`, so the solver was counted twice.

### 8. Withdrawn: there is no unaccounted time around the linear solve (2026-09-22)

This finding claimed that 0.332 s inside `FEProblem::solve` but outside `KSPSolve` was
instruction-free allocation and first-touch page fault cost. It was wrong, and the whole of it is
superseded by finding 14. The subtraction that produced the figure omitted a term. Kept as a heading
only so that references to "finding 8" elsewhere resolve.

### 14. The "unaccounted" time was PCSetUp, and PETSc events do not nest the way I assumed (2026-09-22)

`PCSetUp` is a sibling of the `KSP_Solve` event, not a child of it. So
`SNESSolve - KSPSolve - SNESJacobianEval - SNESFunctionEval` does not leave a remainder to explain:
it leaves `PCSetUp`, BoomerAMG's setup, which was logged in plain sight the whole time.

Verified across five runs spanning two builds, three problem sizes and two allocator configurations,
where the two agree to within one percent:

| run                    | unaccounted | `PCSetUp` |
| ---------------------- | ----------- | --------- |
| serial, refinement 4   | 0.2432      | 0.2421    |
| oprof, refinement 4    | 0.2083      | 0.2073    |
| oprof, huge pages      | 0.2082      | 0.2070    |
| oprof, no malloc trim  | 0.1946      | 0.1941    |
| oprof, refinement 5    | 1.8719      | 1.8648    |

The callgrind result that looked so striking was never in tension with this. `PCSetUp` is 1.86 G
instructions nested inside the `KSPSolve` *function's* 29.65 G, so the three child functions summing
to 100.00% of instructions inside `SNESSolve` is exactly what should be expected. The profile was
consistent with the event log all along; only the subtraction was wrong.

Two conclusions drawn from the bad subtraction are withdrawn. There is no instruction-free cost to
find, so the page fault story in the old finding 8 is gone: testing it directly with
`GLIBC_TUNABLES=glibc.malloc.hugetlb=1` moved nothing (150,873 faults to 148,575, time 0.208 to
0.207), and raising the malloc trim and mmap thresholds cut faults 21% while moving the time 6%, the
6% tracking a genuine small speedup in BoomerAMG setup rather than anything else. And `perf` is not
needed to diagnose any of it, so the `perf_event_paranoid` level is not a blocker on this benchmark.

The second withdrawal is more consequential. The per-iteration comparison in this file was computed
as `(KSPSolve - PCSetUp) / iterations`, which double-removes the setup. `KSPSolve` is already the
iteration phase alone. On the current build over 24 ranks at refinement 5 the correct figures are
3.318 s over 85 matvecs against MFEM's 2.772 s over 84 iterations, which is 0.0390 s against 0.0330 s
per iteration, a factor of **1.18x**. The 1.03x reported earlier is wrong, and the original handoff's
1.26x was nearer the truth than the refutation of it.

That leaves a genuine and unexplained result: libMesh's `PCApply` alone is 3.006 s, larger than the
whole of MFEM's PCG solve at 2.772 s, and both backends link the same
`/opt/petsc/lib/libHYPRE-3.1.0.so`. Whatever is different is in the hierarchy or in how it is applied,
not in the library. AMG operator complexity is 1.59 against MFEM's 1.56, which is close but not
identical, and the operators genuinely differ because constraints are eliminated asymmetrically on the
libMesh side (finding 6). This is now the largest single item in the comparison and it has never been
investigated.

**Rule for anyone differencing PETSc events.** They do not partition their parent. `PCSetUp` sits
outside `KSP_Solve`; `SNESComputeJacobian` does work before `PetscLogEventBegin(SNES_JacobianEval)`
and after the matching end, so the function and the event cover different regions. Never conclude
that a remainder is unexplained work without first checking whether a logged sibling accounts for it,
and never mix an event decomposition with a function decomposition from a profiler.

### 9. The COO preallocation cost is a benchmark artifact, and COO is the right interface (2026-09-22)

An earlier version of this file proposed replacing the COO sparsity handoff with the CSR that
`DofSpace::setupSparsity` already holds, on the grounds that PETSc re-sorts an already-sorted
triplet list. That recommendation was wrong on two counts and has been withdrawn.

COO is PETSc's recommended interface for assembling on a device, and it is the reason the Kokkos
backend can assemble at all. `MatSetPreallocationCOO` takes the triplets once, builds the nonzero
structure and an internal permutation from COO position to storage position; each later assembly
hands over a flat value array and `MatSetValuesCOO` applies that permutation, on device for a device
matrix type, with no per-entry host-side insertion. That is exactly what a Kokkos, CUDA or HIP
backend needs, and it is why `Matrix::close` can submit `_val` positionally. Replacing it with a CSR
handoff would fight the programming model and give up the GPU path.

The cost is paid once per run, not per assembly: `Matrix::create` returns early on `_is_alloc`. This
benchmark is `Steady` with `solve_type = LINEAR` and performs exactly one Jacobian assembly, so
`MatSetPreallCOO` is called once and `MatSetValuesCOO` once, which divides a fixed startup cost by
one. A transient or Newton run amortizes it over every assembly, where the recurring cost is
`MatSetValuesCOO` at 0.026 s serial and 0.047 s on 24 ranks.

That is context, not grounds for dismissal, and an earlier version of this file drew the wrong
conclusion from it. Comparison benchmarks that arrive from users are commonly steady solves shaped
like this one, even though transients are what engineering analysis actually runs, so one-time costs
are disproportionately what a user sees and reports. Weighting them more heavily than their share of
a transient is the right call for this class of work. The 0.256 s is a legitimate target.

The sub-claim that PETSc sorts input that arrived sorted may well be true, since the triplets are
derived from libMesh's per-row-sorted pattern and contain no duplicates, but no alternative was ever
measured and PETSc has no documented way to be told the input is already ordered. Treat it as
unverified speculation, not a plan.

Two small items do survive, and both are memory hygiene rather than a format change, which makes
them relevant to finding 8 rather than to the preallocation time:

`Matrix::create` builds full `std::vector<PetscInt>` duplicates of both index arrays purely to get
contiguous pointers for the PETSc call (`KokkosMatrix.K:49-50`), transiently doubling index memory
at the moment the matrix is being allocated. `Array` already exposes `hostData()`, which
`Matrix::close` uses for the value array, so the copies appear to be avoidable.

`_row_idx` is read only inside `create()`, to build the triplets, yet is retained for the life of
the run; `close()` uses only `_val`, and `find()` and `zero()` use `_col_idx` and `_row_ptr`. At
roughly 2.8 million nonzeros per rank with 8-byte `PetscInt` that is about 22 MB per rank held for
nothing, with another 45 MB transient in the two copies above.

Neither has been implemented or measured.

### 10. Two thirds of solve instructions are hypre, and 7% are OpenMP guards (2026-09-21)

From the same callgrind profile, by instruction count inside `SNESSolve`:
`hypre_BoomerAMGRelaxHybridGaussSeidel_core` 34.05%, `hypre_CSRMatrixMatvecOutOfPlaceHost` 14.09%,
the `VectorKernelGrad::computeJacobianInternal` lambda 5.25%, and libgomp's `omp_get_level` 6.88%,
which is 3.93 G instructions spent deciding whether a loop is already inside a parallel region.

PETSc is configured `--with-openmp=1` and hypre guards its loops accordingly, so a single-threaded
run pays those checks for nothing. This is not a parity lever: both backends link the same
`/opt/petsc/lib/libHYPRE-3.1.0.so`, so MFEM pays it too and the ratio would not move. It is an
absolute win available to both, and it would need a hypre built without OpenMP to collect.

### 11. MFEM runs legacy assembly here, and it is the only level available to this integrator (2026-09-22)

`MFEMProblemSolve` exposes `assembly_level` as a `MooseEnum` over `legacy full element partial none`
defaulting to `legacy`, and `mfem_cantilever.i` does not set it. So the MFEM side assembles element
matrices through `ElasticityIntegrator::AssembleElementMatrix` and inserts them into a
`SparseMatrix`, then builds the `HypreParMatrix`. That is a host full-assembly path, directly
comparable in kind to the Kokkos backend's, and BoomerAMG requires an assembled matrix on both
sides, so neither backend can use a matrix-free level here.

`assembly_level=full` is not merely unused, it aborts: `AssembleEA(...) is not implemented for this
class`. `bilininteg_elasticity_ea.cpp` implements `AssembleEA` for `ElasticityComponentIntegrator`,
the single `(i,j)` block integrator, not for the `ElasticityIntegrator` that
`MFEMLinearElasticityKernel` uses. `partial` would work, since `ElasticityIntegrator::AssemblePA`
exists, but yields a matrix-free operator BoomerAMG cannot precondition. The name "legacy" is
misleading: for this integrator it is the production path.

The two element kernels are structurally different, which is the substance of the remaining assembly
gap. MFEM works entirely in the scalar degree of freedom space: for HEX8 `dshape` and `gshape` are
8 by 3, not 24 by 3. Per quadrature point it forms `pelmat = gshape gshape^T`, an 8 by 8 scalar Gram
matrix, once, and reuses it across all three diagonal blocks; it applies the whole lambda term as a
single rank-one update through `AddMult_a_VVt`; and it fills the mu cross terms from scalar gradients
indexed by component. It never evaluates constitutive work per vector degree of freedom pair.
Roughly 12 thousand multiplies per element against MOOSE's 23 thousand after finding 4a, which is
consistent with 0.834 s against MFEM's 0.783 s, though note that MFEM's figure covers its sparsity
construction as well, so the element kernels alone are closer than that.

This means finding 4a closed a memory traffic gap within a per (i, j, qp) formulation, and what is
left is a difference in formulation that no further micro-optimization of those loops reaches. The
handoff's original phrasing, that MFEM exploits a vector Lagrange shape function having one nonzero
component, understates it: MFEM exploits the tensor structure of component times scalar basis to
work in blocks, which is strictly more than skipping structural zeros.

### 12. MFEM's non-legacy full assembly is CSR-like, not COO-like (2026-09-22)

Recorded because it is the natural comparison to the Kokkos backend's COO handoff, though it does
not apply to this benchmark's integrator.

`FABilinearFormExtension::Assemble` first runs the element-assembly extension to produce `ea_data`,
a flat E-vector of element matrices, then for a continuous space calls
`ElementRestriction::FillSparseMatrix`, which builds CSR arrays directly: `FillI` counts entries per
row and prefix-sums them into the row pointers and returns the nonzero count, `J` and `Data` are
allocated at that size, and `FillJAndData` writes column indices and sums element contributions
straight into the value array. Column indices are sorted afterwards only if the form asks
(`sort_sparse_matrix`). There is no triplet list and no sort-derived permutation anywhere.

Deduplication, where several elements contribute to the same entry, is handled by the restriction
maps the space already owns, `offsets`, `indices` and `gather_map`, with a `GetMinElt` tie-break that
attributes each pair to the lowest-numbered element sharing it. All of it runs in device-capable
`mfem::forall` kernels.

So both frameworks precompute a map from element-local positions to matrix storage once and then
submit values cheaply, but they get the map from different places. PETSc's COO derives a permutation
by sorting a caller-supplied triplet list; MFEM reuses the element restriction that its E-vector and
L-vector machinery already requires, so it needs no sort. MOOSE-Kokkos has no equivalent restriction
object, which is part of why COO is the natural interface for it rather than an accident.

Note also that MFEM's path keeps `ea_data` alive, an E-vector sized by the element count times the
square of the element degree of freedom count, which is not obviously cheaper in memory than COO
index arrays. Given finding 8, any comparison of the two on footprint would have to measure rather
than assume.

### 13. What PETSc's COO actually does, and what the Kokkos backend asks of it (2026-09-22)

Read from the `petsc` submodule at v3.25.4, which matches the installed library, so this is verified
rather than inferred from the API.

**Setup.** `MatSetPreallocationCOO_MPIAIJ` (`src/mat/impls/aij/mpi/mpiaij.c`) sorts the triplets by
row with a permutation array through `PetscSortIntWithIntCountArrayPair`, separates ignored, local
and remote entries, sets up the communication for the remote ones, and builds two arrays: `perm`,
the permutation from segment position to input position, and `jmap`, the segment offsets giving which
input entries land on each unique nonzero.

**Per assembly.** `MatSetValuesCOO_SeqAIJ` (`src/mat/impls/aij/seq/aij.c`) is then just

```
for (i = 0; i < Annz; i++) {
  PetscScalar sum = 0.0;
  for (j = jmap[i]; j < jmap[i + 1]; j++) sum += v[perm[j]];
  Aa[i] = (imode == INSERT_VALUES ? 0.0 : Aa[i]) + sum;
}
```

a segmented sum over a permuted gather. No sorting happens at assembly time; the ordering was fixed
once at setup. There are no atomics and no contention, and the flop count is one add per input
entry, which is the minimum.

**For MATHYPRE.** `MatSetPreallocationCOO_HYPRE` (`src/mat/impls/hypre/mhypre.c:2413`) creates an
internal AIJ agent matrix, runs the AIJ COO machinery on it, copies the sparsity into the hypre
IJMatrix, and then aliases the agent's value array to the hypre matrix's own
(`MatHYPRE_AttachCOOMat`, "Alias cooMat's data array to IJMatrix's"). The source comment is explicit:
"cooMat is only a way to reuse PETSc COO code." So there is no per-assembly copy into hypre, which is
why finding 1 saw no assembly penalty from the matrix type.

**Why COO is the device-preferred interface.** Not the triplet storage, but that it permits
duplicates. That lets a caller emit element contributions into consecutive slots in element order,
with no search for storage position, no conflict between elements sharing a degree of freedom, and no
knowledge of the matrix's internal block splitting, and leaves the reduction to the library.

**What the Kokkos backend does instead.** `Matrix::_val` is sized at the unique nonzero count in CSR
order, the header calling them "CSR vectors on device". Each contribution locates its slot with
`find()`, a binary search over the row's column range, and accumulates with `::Kokkos::atomic_add`.
So the backend performs the reduction itself, on device, with a search and atomics, and then presents
the already-reduced result to PETSc as a triplet list. It forgoes what COO is for while still paying
for the setup sort that COO needs. Entries are duplicate-free except on shared rows, where several
ranks contribute to the same entry and the segments are genuinely longer than one, so the parallel
reduction and its communication are real work; the interior majority is a permuted gather of values
that are already in order.

That is a conceptual mismatch visible by reading, independent of what it costs, and it is the reason
to look at it.

**Two coherent designs, opposite in spirit.** Neither has been prototyped.

Use COO as intended: size `_val` at the contribution count, elements by element-local pairs, in
element order; delete `find()` and the atomics; let PETSc reduce. Removes the search, removes the
atomics, and makes the permutation earn its keep. Costs value traffic, roughly 576 against 243
doubles per element for HEX8 with a vector unknown, which is the same trade MFEM makes with
`ea_data` in finding 12, and traffic is what finding 4a identified as the limiter.

Or own the reduction honestly: keep the unique-nonzero `_val`, but precompute the contribution to
slot map at setup so assembly is a direct indexed write with no search, and if that map is ordered by
slot, a segmented reduction with no atomics either. This keeps the low traffic value array and moves
the memory from values to indices. Note what it is: building PETSc's own COO permutation ourselves,
targeting our CSR rather than PETSc's.

**On determinism.** An earlier version of this file called the non-reproducibility of atomic assembly
a bug. It is not. Floating point summation is not associative and a parallel assembly reorders it;
that is expected, and libMesh's debug-only `snesmf_reuse_base` check simply assumes more than a
parallel atomic assembly can offer. Nor does PETSc's fixed ordering cost flops in the hot path: the
segmented sum has the same add count as atomics would and avoids contention. The determinism is paid
for entirely by the setup sort. Which is the point worth carrying: the sort is not buying us anything
we need, and for a single-assembly steady solve we pay all of it to make exactly one assembly
deterministic.

**Two things that look like waste and are not.** Recorded so they are not "cleaned up".
`Matrix::create` copies both index arrays into `std::vector` before the PETSc call because
`MatSetPreallocationCOO` is documented as free to modify them in place, translating them to global
numbering, while `_col_idx` and `_row_ptr` are read on device by `find()` for the life of the run.
And `_row_idx`, though unused after `create()`, is a reference-counted handle onto
`DofSpace::_sparsity.row_idx`, which the degree of freedom space owns for the whole run, so releasing
it from the matrix frees nothing. The only change made was to allocate `_val` after the preallocation
call and scope the copies, so that the value array and the index copies are not held at once.

### 15. CF-relaxation was the whole of the PCApply gap (2026-09-22)

PETSc calls `HYPRE_BoomerAMGSetRelaxOrder(1)`, asking BoomerAMG to relax coarse and fine points in
separate passes. `mfem::HypreBoomerAMG` never calls that function, so hypre's own default applies on
the MFEM side. The two backends were therefore smoothing differently for the whole history of this
benchmark, even though coarsening, interpolation type, interpolation truncation, strength threshold
and aggressive coarsening levels had all been matched deliberately. Relaxation ordering was simply
never on the list.

Measured on 24 ranks at refinement 5, means of two or three runs:

| configuration                                     | `KSPSolve` | `PCApply` | matvecs | s/iteration |
| ------------------------------------------------- | ---------- | --------- | ------- | ----------- |
| default: symmetric-SOR/Jacobi with CF-relaxation  | 3.380      | 3.060     | 85      | 0.03977     |
| relax type 8 (`l1scaled-SOR/Jacobi`) only         | 3.426      | 3.114     | 86      | 0.03984     |
| `-pc_hypre_boomeramg_no_CF` only                  | 2.656      | 2.315     | 84      | 0.03162     |
| MFEM                                              | 2.772      | -         | 84      | 0.03300     |

Two things worth taking from the middle row. The relaxation *type* is not the lever: MFEM's
`SetDefaultOptions` chooses `relax_type = 8` where PETSc defaults to symmetric relaxation, but
matching it changes nothing measurable. And the ordering is: dropping CF-relaxation removes 23% of
each preconditioner application, leaves the iteration count unchanged, and takes the per-iteration
cost slightly below MFEM's.

`maxZ` is `-2.9248578006624e-10` against the reference `-2.9248578006653e-10`, agreeing to twelve
significant figures, which is what a changed preconditioner on a solve converged to a tolerance
should give.

This is a matching change, not a recommendation for MOOSE at large. CF-relaxation is often useful on
harder problems, which is presumably why PETSc enables it; on this operator it buys no iterations at
all. Whether MOOSE's default should change needs evidence from problems other than this one.

**How it was found**, since the route generalizes: `-ksp_view` prints every BoomerAMG parameter PETSc
has set, including "Using CF-relaxation" and the relax types, and MFEM's choices are readable in
`HypreBoomerAMG::SetDefaultOptions` and `SetSystemsOptions` in `linalg/hypre.cpp`. Diffing those two
lists is the way to check that two backends are really solving the same way. Iteration count matching
is not sufficient evidence that they are: 85 against 84 looked close enough to be ignored for two
sessions while a 23% difference in smoother cost hid behind it.

### 16. Audit of the rest of the BoomerAMG configuration: matched, and the residual difference is the operator (2026-09-22)

Prompted by finding 15, where an unmatched parameter hid for two sessions behind iteration counts of
85 against 84. The method: enumerate every `HYPRE_BoomerAMGSet*` call each side makes and diff them.
PETSc applies 33 of them in `ihypre.c`; `mfem::HypreBoomerAMG` applies 12 in
`HypreBoomerAMG::SetDefaultOptions` and `SetSystemsOptions` and leaves the rest at hypre's defaults.
`-ksp_view` prints PETSc's values, which is how to read them without tracing the source.

Matched, either because the input sets both or because PETSc's value coincides with the hypre default
MFEM leaves alone: coarsening HMIS, interpolation ext+i, `P_max` 4, one level of aggressive
coarsening, strength threshold 0.7, cycle type V, one sweep down, up and coarse, 25 maximum levels,
interpolation truncation factor 0, maximum row sums 0.9, local measure type, one path for aggressive
coarsening, coarsest grid 9, Gaussian elimination on the coarsest level, relaxation weight 1.0, and
aggressive interpolation type 4, which is PETSc's host default and also hypre's.

Three differences survive and none is a configuration problem.

Relaxation type differs, PETSc's symmetric-SOR/Jacobi against MFEM's `relax_type = 8`, and it was
measured in finding 15 to change nothing.

`min_coarse_size` is 1 for PETSc against hypre's default of 0. Immaterial: both hierarchies bottom out
at between 6 and 9 rows regardless.

MFEM passes an explicit degree of freedom to function map through `SetDofFunc` while PETSc relies on
hypre's default assumption. Verified equivalent by experiment rather than by reading: with
`-pc_hypre_boomeramg_numfunctions 1` the libMesh side takes 231 matvecs and 11.25 s, against 84
matvecs and 6.50 s with 3. The by-component treatment is therefore working, so hypre's default
assumption agrees with libMesh's `LAGRANGE_VEC` global ordering.

**The hierarchies confirm it.** At refinement 5, both build ten levels from an identical fine
operator, 839,619 rows and 65,119,689 nonzeros on each side, and grid complexity is 1.322 against
1.345.

| level | libMesh rows | MFEM rows | libMesh nonzeros | MFEM nonzeros |
| ----- | ------------ | --------- | ---------------- | ------------- |
| 0     | 839,619      | 839,619   | 65,119,689       | 65,119,689    |
| 1     | 176,717      | 189,958   | 15,792,247       | 16,783,902    |
| 2     | 59,987       | 64,669    | 11,524,483       | 11,451,569    |
| 5     | 2,110        | 2,122     | 452,398          | 412,988       |
| 9     | 9            | 6         | 73               | 36            |

So coarsening and interpolation are effectively matched, and what remains of the divergence traces to
the operators being different rather than to any parameter. The level 0 row sums say so directly:
libMesh spans `[-9.596e-06, 1.000e+00]` and MFEM `[-5.406e+08, 3.509e+09]`, which is finding 6, the
asymmetric elimination of essential constraints, measured afresh on the current build.

That makes finding 6 the last known solver-side difference. It is no longer a performance target,
since the libMesh solve is now the faster of the two, but it remains a correctness point: `-ksp_type
cg` is being applied to a matrix that is not symmetric, and it is the reason two otherwise identical
discretizations produce hierarchies that are not quite the same.

### 17. The COO preallocation cost is irreducible O(n) work, so the only lever is fewer entries (2026-09-22)

Profiled with callgrind on the `oprof` build, collection toggled on at `MatSetPreallocationCOO`,
serial at refinement 4. Of 470.8 M instructions inside the call:

| component                                              | share | what it is |
| ------------------------------------------------------ | ----- | ---------- |
| `MatSetPreallocationCOO_SeqAIJ`, self                  | 63.3% | the O(n) bookkeeping: build `perm`, detect sortedness, count duplicates, build `jmap` and row pointers |
| `__memset_avx2_unaligned_erms`                         | 15.2% | zeroing freshly allocated arrays; 71.5 M writes, 58% of all writes in the region |
| `MatHYPRE_IJMatrixCopyIJ`                              | 9.0%  | copying the pattern into hypre's IJMatrix |
| hypre ParCSR construction and the rest                 | ~12%  | |

**No sort appears anywhere in the profile.** `MatSetPreallocationCOO_SeqAIJ` reads
`/* Sort by row if not already */ if (!isorted) PetscSortIntWithIntCountArrayPair(...)`, so PETSc
detects that the input is already ordered and skips it. The sparsity from `DofSpace::setupSparsity`
is sorted within each row, PETSc notices, and we already get the benefit for free.

That kills the design idea this was heading towards. Telling PETSc the COO is sorted, by a new option
or API in the idiom of `MAT_SORTED_FULL`, would buy nothing: it is already inferred. And it is not
even available in parallel, where `MatSetPreallocationCOO_MPIAIJ` sorts unconditionally and
deliberately: it first rewrites the row indices, shifting owned rows into a negative range and ignored
entries to `PETSC_INT_MIN`, so that one sort simultaneously partitions the array into ignored, local
and remote sections and orders each. The sort is doing double duty as the partition, so it cannot
simply be skipped on a sortedness hint.

What is left is O(n) work in n, the number of point entries: one bookkeeping pass, one zeroing pass,
one structural copy. There is no algorithmic slack. **The only real lever is to reduce n.**

Which makes the block structure the answer rather than a curiosity. The unknown has three components
per node, so every node-to-node coupling implies all nine component pairs: the point pattern carries
65.1 M entries at refinement 5 where the block pattern carries about 7.2 M. Every one of the four
components above scales with n, so describing the matrix in blocks would cut all of them by roughly
nine, taking 0.256 s to something near 0.03 s, and would cut the index memory by the same factor.

It is blocked upstream, not by anything in MOOSE. PETSc implements COO for AIJ, AIJKokkos, HYPRE, IS
and a `_Basic` fallback its own comment calls very slow, but not for BAIJ; and hypre's ParCSR is
point-wise, using `num_functions` rather than blocks, so BAIJ and MATHYPRE do not combine at all.

Two smaller levers, both inside PETSc. The MATHYPRE agent-matrix design costs the 9% structural copy
that building hypre's IJMatrix directly would avoid. And the 15% spent zeroing arrays that are then
immediately overwritten looks avoidable.

**So there is nothing for MOOSE to do here, and open question 1 is retired as a MOOSE target.** The
0.256 s is PETSc performing necessary O(nnz) work while already skipping the only step that could have
been skipped. Pursuing it means either an upstream block-COO path or accepting the cost.

One caveat on the evidence: this profile is serial, which takes the `SeqAIJ` path. Production runs at
24 ranks use the MPIAIJ agent, whose unconditional sort does not appear here, so the parallel
composition includes a term this profile does not show. Re-profiling under MPI would settle how large
that term is; the conclusion that the remainder is O(n) work does not depend on it.

### 18. Setup decomposed: libMesh's sparsity construction is the dominant cost (2026-09-22)

Callgrind over the whole run, `oprof` build, serial at refinement 4, 71.77 G instructions total.
Inclusive costs:

| section                                | instructions | share of run | notes |
| -------------------------------------- | ------------ | ------------ | ----- |
| `EquationSystems::init`                | 3.678 G      | 5.12%        | |
| -- `DofMap::compute_sparsity`          | 2.872 G      | 4.00%        | 78% of the section |
| ---- `SparsityPattern::Build::operator()` | 2.781 G   | 3.87%        | 97% of compute_sparsity |
| ------ `sorted_connected_dofs`         | 0.367 G      | 0.51%        | 13% of the Build functor |
| -- `DofMap::distribute_dofs`           | 0.550 G      | 0.77%        | 15% of the section |
| -- matrix preallocation                | ~0.071 G     | 0.10%        | `init_matrices` minus `compute_sparsity` |
| `FEProblem::initKokkos`                | 1.222 G      | 1.70%        | |
| -- `Kokkos::Assembly::init`            | 0.437 G      | 0.61%        | 36% of the section |
| -- `Kokkos::Mesh::update`              | 0.407 G      | 0.57%        | 33% of the section |
| -- `DofSpace::setupSparsity`           | 0.165 G      | 0.23%        | 13.5% of the section |

Mesh construction is not where the setup deficit lives. libMesh spends 0.510 s in the generators,
0.122 s deleting remote elements and 0.036 s preparing, totalling 0.668 s against MFEM's 0.684 s in
`MFEMFileMesh::buildMesh`. The whole 0.44 s deficit is `EquationSystems::init` at 0.218 s against
MFEM's 0.004 s, plus `initKokkos` at 0.217 s with no MFEM analogue.

**MFEM has no up-front sparsity pattern at all.** `BilinearForm::Assemble` allocates the
`SparseMatrix` lazily, `if (mat == NULL)`, assembles into dynamic per-row structure and finalizes to
CSR afterwards. libMesh computes the pattern first, preallocates exactly, then assembles into fixed
storage. So MFEM's 0.004 s is not MFEM being quicker at the same work; it is MFEM deferring the work
into its 0.74 s of assembly. Comparing the setup phases alone will always flatter MFEM for this
reason, and the honest comparison is setup plus assembly: 1.97 s against 1.44 s.

That also reinstates the first handoff's observation about multiple representations of the same graph,
now with numbers. The pattern is materialized three times: libMesh's `SparsityPattern::Graph`, a
`vector<vector<dof_id_type>>`, in `EquationSystems::init`; the Kokkos device CSR derived from it in
`initKokkos`; and PETSc's COO permutation plus hypre's IJMatrix in the first Jacobian, finding 17.

Two guesses made before this profile were wrong and are recorded so they are not repeated.
`DofSpace::setupSparsity` does not dominate `initKokkos`; it is 13.5% of it, behind both the Kokkos
assembly tables and the Kokkos mesh update. And removing libMesh's matrix preallocation for Kokkos
systems, on the grounds that COO re-preallocates later, is not worth doing: it is 2% of
`EquationSystems::init`, about 0.005 s.

### Development areas this identifies

**Expand a node-level graph instead of building a degree-of-freedom-level one.** This is the largest
single lever in setup. `SparsityPattern::Build` inserts coupled degrees of freedom row by row with
sorted insertion, so its cost scales with the number of point entries. For a `LAGRANGE_VEC` unknown
with three components the degree-of-freedom graph is the node graph expanded by a three-by-three
block, so computing the node coupling once and expanding it would cut the insertion work by roughly
nine. Unlike the block idea for the COO path in finding 17, nothing about hypre's point-wise storage
blocks this: it is internal to libMesh's pattern construction, and libMesh is in scope. 2.872 G
instructions, 4% of the whole run, is the prize.

**Emit a flat CSR from libMesh.** `SparsityPattern::Graph` is a vector of per-row vectors, so the
Kokkos backend must transform it into contiguous arrays. Having libMesh produce a flat CSR would
remove a pass and hand the device backend what it actually wants. Smaller than the item above, at
0.165 G, but it composes with it.

**Attribute `Kokkos::Assembly::init`.** At 0.437 G it is the largest MOOSE-owned item in setup and
there is no obvious reason for it to be that large, since the shape tables are per element type
rather than per element. Per-element material property indexing is the likely candidate and has not
been checked.

### 19. Itemizing the residual: there is no single hidden item (2026-09-22)

The standing measurement left roughly 0.9 s of setup and execute work unaccounted. Itemized, it is
not one thing, and the search for a smoking gun came up empty. That is the result.

First, the PerfGraph was run at `Outputs/perf/level=9` to check whether MOOSE already registers finer
sections that the usual level of 5 simply does not print. It registers 211 sections at level 9 and the
picture is unchanged: nothing with a self time above 0.02 s appears that was not already visible. So
the gap is missing instrumentation and diffuse cost, not hidden detail.

Second, the instruction profile shows nothing unaccounted either.
`libmesh_petsc_snes_jacobian` is 35.16% of the run, identical to `SNESComputeJacobian`, so libMesh's
callback wrapper costs nothing measurable. An earlier reading of this profile claimed a 2.256 G gap
between that wrapper and MOOSE's `computeJacobian`; that was a misread of a fragmented demangled name
in `callgrind_annotate` output and there is no such gap.

Full itemization, 24 ranks, refinement 5, wall 6.03 s:

| item | s |
| ---- | --- |
| mesh: `ExecuteMeshGenerators` | 0.516 |
| mesh: `deleteRemoteElems` | 0.131 |
| mesh: prepare and update | 0.036 |
| solve: `KSPSolve` | 2.618 |
| solve: `PCSetUp` | 0.315 |
| assembly: `KokkosKernel` | 0.816 |
| assembly: `MatSetPreallocationCOO` | 0.253 |
| assembly: `DofMap::compute_sparsity` | ~0.156 |
| assembly: `MatSetValuesCOO` | 0.035 |
| assembly: `DofSpace::setupSparsity` | ~0.028 |
| assembly: libMesh matrix preallocation | ~0.004 |
| other: `initKokkos` less `setupSparsity` | 0.183 |
| other: `FEProblem::solve` self beyond the KSP | 0.096 |
| other: `KokkosReinit`, four calls | 0.098 |
| other: `computeUserObjects` | 0.057 |
| other: `projectSolution` | 0.056 |
| other: `EquationSystems::Init` less `compute_sparsity` | 0.044 |
| other: `Console::outputStep` | 0.023 |
| **named total** | **5.465** |
| **residual** | **0.565** |

By bucket: mesh 0.683, solve 2.933, assembly 1.292, other 0.557.

Of the 0.565 s residual, about 0.17 s is process startup and teardown outside both the setup and
execute phases, where MOOSE is *faster* than MFEM's 0.41 s. The remaining roughly 0.4 s is spread
across the two hundred-odd PerfGraph sections that are each below 20 ms, plus glue that carries no
timer at all. No single item in it is worth attacking.

**What that says about the comparison.** MFEM's executioner does two things, `FormSystem` and
`SolveSystem`, which together account for all 3.86 s of it. MOOSE's does dozens: the
`SteadyBase`/`FixedPointSolve`/`FEProblemSolve` layering, auxiliary system evaluation, user objects,
initial condition projection, console output, and per-evaluation reinitialization. Roughly 0.4 to 0.5 s
of the remaining deficit is that generality rather than any inefficiency, and closing it would mean
removing capability rather than making anything faster.

So after this itemization the remaining 1.03 s deficit reads: about 0.54 s structural work in the
assembly path, about 0.45 s of framework generality with no MFEM counterpart, and a 0.14 s advantage
to libMesh in the solve. The first is the only part with an identified mechanism to attack.

### 20. The ratio depends strongly on rank count, and libMesh scales better (2026-09-22)

Every other figure in this file is from 24 ranks at refinement 5. That is one point in a
two-dimensional space and the ratio is not flat across it. Strong scaling at refinement 5, one run per
point so treat the absolute numbers as plus or minus 0.1 to 0.3 s at the lower rank counts:

| ranks | libMesh | MFEM  | ratio  | deficit | deficit x ranks |
| ----- | ------- | ----- | ------ | ------- | --------------- |
| 6     | 17.23   | 11.95 | 1.442x | +5.28   | 31.7            |
| 12    | 9.72    | 7.57  | 1.284x | +2.15   | 25.8            |
| 24    | 6.07    | 4.86  | 1.249x | +1.21   | 29.0            |

Two conclusions, one of which contradicts an argument made earlier in this file.

**The deficit is per-rank work, not fixed overhead.** Deficit times rank count is roughly constant at
26 to 32 rank-seconds, so the gap is a fixed quantity of O(N) work spread over the ranks rather than
serial setup that would stay constant as ranks increase. The claim that what remains is fixed cost
which amortizes away in a transient is therefore wrong as stated: only the part that runs once per
run, the sparsity pattern and the COO preallocation, amortizes. The element kernel does not.

**libMesh scales better than MFEM here, and that is why the ratio improves with rank count.** The
attribution of that to MFEM's serial mesh refinement, below, is corrected by finding 21: the mesh is
where the cost appears but removing it does not recover the scaling. From 6
to 24 ranks libMesh speeds up 2.84x against MFEM's 2.46x, so 71% parallel efficiency against 61%. The
prediction that the `2 x num_procs` collective scatter loop in `DofSpace::setupSparsity` would punish
us at scale does not show up by 24 ranks. MFEM's serial refinement to full size on every rank before
partitioning, with `parallel_refine` at 0, is the likely reason its efficiency is lower, and libMesh's
`DistributedRectilinearMeshGenerator` being O(N/P) should keep the trend running our way.

**Practical consequence.** 1.249x is the most favourable number in the measured range, not a
representative one. Anyone running this benchmark at small rank counts will report something closer to
1.44x, which is where the whole session started. Nothing below 6 ranks has been measured on the MFEM
side at all, so the low-rank end of the curve is unknown and is exactly where a workstation user would
sit.

### 21. MFEM's parallel_refine fixes its mesh cost and buys it nothing (2026-09-22)

Finding 20 showed MFEM scaling worse than libMesh, and its `MFEMFileMesh::buildMesh` growing with rank
count rather than shrinking: 0.509, 0.584 and 0.647 s at 6, 12 and 24 ranks. The mechanism is real, it
refines serially to full size on every rank before partitioning, and `parallel_refine` on `MFEMMesh`
refines after partitioning instead. Tested with `refinement=2 Mesh/parallel_refine=3`, which reaches
the same 262,144 elements, one run per point:

| 24 ranks       | serial refine | `parallel_refine=3` |
| -------------- | ------------- | ------------------- |
| `buildMesh`    | 0.647         | 0.038               |
| `FormSystem`   | 0.727         | 1.011               |
| `SolveSystem`  | 2.971         | 3.265               |
| iterations     | 84            | 85                  |
| wall           | 4.86          | 4.82                |

At 6 ranks: 11.95 to 12.11 s, `buildMesh` 0.509 to 0.063, iterations 84 to 71 while the solve stayed at
8.45 s, so the per-iteration cost worsened enough to absorb thirteen fewer iterations.

Mesh construction becomes 17 times cheaper and the saving is cancelled by worse assembly and worse
solve. The cause is partition quality: partitioning 512 coarse elements and then refining gives a
different decomposition from partitioning 262,144 directly, so communication rises in both phases, and
BoomerAMG's coarsening is partition dependent, which is why the iteration count moves. Scaling barely
changes, 2.51x from 6 to 24 ranks against 2.46x, so MFEM remains near 62% efficiency against libMesh's
71%.

**Correction to finding 20.** That finding concluded the mesh was essentially the whole scaling
difference, by subtracting `buildMesh` from MFEM's totals. The subtraction was arithmetically right and
practically wrong: removing the component does not remove the cost, because partition-quality cost
takes its place and scales just as badly. A decomposition is not a causal model. This is the third
instance of that error in this file, after treating instruction share as time share and treating event
subtraction as attribution.

**Consequence for fairness.** Leaving `parallel_refine` at 0 does not handicap MFEM, so the comparison
stands. The reason recorded in `mfem_cantilever.i`, that enabling it changes what is being compared, is
defensible but the measured reason is stronger: it trades mesh time for partition quality at no net
gain.

### 22. The production material chain costs nothing measurable here (2026-09-23)

Measured on the tree of `9e13bce73b0` ("Move the elasticity cantilever onto the production objects"),
libMesh from `libmesh/installed`, `Executable Timestamp` Tue Sep 22 23:35:34 2026. 24 MPI ranks, one
thread per rank, `refinement=5`, fast configuration, file output off.

`kokkos_cantilever.i` previously used `KokkosLinearElasticity`, which baked isotropic elasticity into
the kernel and took the two Lame parameters as `Real` material properties. It now runs
`KokkosComputeIsotropicElasticityTensor`, `KokkosComputeSmallStrain` and
`KokkosComputeLinearElasticStress` with `KokkosVectorStressDivergence`, so the stress and the tangent
travel through material properties.

| quantity            | baked-in kernel | production chain      |
| ------------------- | --------------- | --------------------- |
| wall (s), median    | 6.03            | 6.07                  |
| runs                | 3               | 6.18 / 6.07 / 6.04    |
| `maxZ`              | -2.924858e-10   | -2.9248578006239e-10  |

The 0.04 s difference is below the 0.15 s repeat-run threshold, and the tip deflection is unchanged.

**Do not generalise this to "material properties are free."** The benchmark is `solve_type = LINEAR`,
so the residual and the Jacobian are each assembled once. The chain stores `Jacobian_mult` as a dense
rank-four property, 648 B per quadrature point, which at 262,144 elements and eight quadrature points
is 1.36 GB, or 57 MB per rank at 24 ranks, written once and read once. A nonlinear problem
reassembling the Jacobian every Newton iteration pays that traffic per iteration, and this measurement
says nothing about that case.

The tangent is constant per block for linear elasticity and is stored per quadrature point only
because a Kokkos material's `constant_on` applies to every property it declares. Tracked as
idaholab/moose#33855; the objects themselves are idaholab/moose#33859.

## Refuted claims and dead ends

Do not re-attempt these without new evidence. Each line says what measurement closed it.

| claim | what measurement showed | when |
| ----- | ----------------------- | ---- |
| The COO sparsity handoff should be replaced with the CSR the Kokkos side already holds, because PETSc re-sorts an already-sorted triplet list | Withdrawn, not measured, and wrong in premise. COO is PETSc's recommended device-assembly interface and is what lets `Matrix::close` submit values positionally; a CSR handoff would give up the GPU path. The cost is also once per run rather than per assembly, and this benchmark performs exactly one Jacobian assembly, so it divides a fixed startup cost by one. See finding 9. | 2026-09-22 |
| Recording the component of each vector shape function and contracting one row of the dense gradient table is enough to speed up the element kernel | Refuted, and a regression. `KokkosKernel` went from 1.139 s to 1.311 s and wall from 6.97 s to 7.22 s. The multiply count fell from nine to three, but the loop still read the whole `Real33` and now also paid a dependent `_components(_rows(i))` load per (i, j, qp). Reducing multiplies without reducing loads loses. The gradient has to be stored compactly for the win to appear; see finding 4a. | 2026-09-21 |
| The PETSc-to-hypre preconditioner wrapper copies vectors on every application and is the leading suspect for a 1.26x per-iteration cost | Refuted twice over. `PCApply` over 85 applications is 2.828 s on AIJ against 2.929 s on a native ParCSR, i.e. no improvement and slightly worse, so the plumbing carries no per-iteration penalty. The stated mechanism also does not exist: PETSc 3.25.4 exposes `VecHYPRE_IJVectorPushVec`, which pushes the array pointer rather than copying. | 2026-09-21 |
| The solve carries a 0.88 s deficit and a 1.26x per-iteration cost | Partly reinstated. The refutation of this was itself wrong: it computed the iteration phase as `KSPSolve - PCSetUp`, double-removing the setup, and reported 1.03x. The correct per-iteration figure is 1.18x, and the solve deficit including `PCSetUp` is 0.58 s. So the original claim was closer to right than its refutation. See finding 14. | corrected 2026-09-22 |
| A 0.33 s cost around the linear solve is instruction-free and follows the memory footprint | Withdrawn. It is `PCSetUp`, a sibling event that the subtraction omitted, agreeing to within 1% across five runs. Direct interventions on page fault cost moved it essentially not at all. See finding 14. | 2026-09-22 |
| Constructing the matrix as `MATHYPRE` from the start, to skip libMesh's AIJ preallocation, is worth pursuing | Not worth it. `MatSetPreallocationCOO_HYPRE` is itself 0.061 s more expensive than the MPIAIJ COO path, and `FEProblem::EquationSystems::Init` barely moved (0.213 to 0.207). A libMesh `PetscMatrix::_mat_type` setter should not be added for this. | 2026-09-21 |
| `KokkosSystem::MatSetPreallocation` duplicates sparsity work libMesh has already done | Half true, and the wrong half was acted on. `DofSpace::setupSparsity` (`framework/src/kokkos/systems/KokkosDofSpace.K:177`) builds the Kokkos CSR *from* `_dof_map.get_sparsity_pattern()`, so the pattern is shared. Only the AIJ preallocation is redundant. | 2026-09-21 |
| `MatConvert`, PETSc AIJ to hypre ParCSR, is a solve bottleneck | 0.104 s. Real, and now recovered by finding 1, but it was never the bottleneck. | 2026-09-20 |
| MOOSE already executes this problem 1.32x faster than MFEM | False, and the product of a stale mixed binary. At matched build and resources execute was 4.88 s against 4.72 s, roughly parity; with matched AMG settings MFEM is ahead. | 2026-09-20 |
| The first handoff's libMesh figures (7.74 s wall, 90 iterations, 3.62 s execute) | Stale mixed binary. A consistent build of the same tree gave 9.11 s and 69 iterations. Not reachable. | 2026-09-20 |
| A mid-session gap of 1.14x | Compared libMesh with aggressive coarsening against MFEM without it. Matched, it was 1.41x without on either side and 1.44x with it on both. | 2026-09-20 |
| "The solve is won" | Same cause as above. With matched settings the solve was 3.94 s against 2.97 s. | 2026-09-20 |
| Kokkos with a distributed mesh is pathological (`uniformRefine` 40.8 s) | Nothing to do with Kokkos. It is the `GhostLowerDElems` relationship manager that any `AuxKernel` registers; see the bug below. Disabling `[AuxKernels]` took distributed refinement from 26.76 s to 9.53 s on an identical mesh, while disabling `[Kernels]` or `[BCs]` changed nothing. | 2026-09-20 |
| The `GeneratedMeshGenerator` partition fix might have degraded partition quality | A genuine win. Reverting it made the run slower, 8.41 s to 10.04 s. | 2026-09-20 |
| Distribute a coarse mesh then refine in parallel, which should scale better | Slower than generating the fine mesh already distributed, in every pairing. Distributed refinement cost 4.66 s against replicated 1.70 s despite fewer elements per rank. | 2026-09-20 |
| Material property lookups in the Kokkos elasticity kernel | 0.049 s of 0.962 s. The framework contraction loop is the cost. | 2026-09-20 |
| Two levels of aggressive coarsening | Worse than one for MFEM, 5.14 s against 4.90 s. | 2026-09-20 |
| GMRES instead of CG on the libMesh side | 186 iterations against 86, and 11.15 s against 7.22 s. | 2026-09-20 |
| Cheaper partitioners, hash table assembly, MetaPhysicL PR #73, libMesh devel for its own sake | Ruled out by measurement in an earlier session. | before 2026-09-20 |

Two settings are worth keeping even though they do not affect the fast configuration:
`Mesh/skip_deletion_repartition_after_refine` is worth having wherever refinement is used
(replicated 2.91 to 1.70 s, distributed 9.71 to 4.66 s), because libMesh otherwise repartitions and
deletes remote elements after every level. And mesh construction is now in libMesh's favour:
`DistributedRectilinearMeshGenerator` builds only each rank's own partition at 0.464 s, while MFEM
refines serially to the full element count on every rank before partitioning, at 0.648 s. MFEM's
`parallel_refine` parameter, left at 0 in this benchmark, would flip that back.

## Open questions, in the order worth attacking

Changes to PETSc and to hypre are as much in scope as changes to MOOSE. That is not a detail of
prioritization: the ranking below is materially different from what it would be if only MOOSE code
could change, and an earlier version of this file twice set aside a large cost on the grounds that it
was not ours or did not move the benchmark ratio. Both are bad reasons on their own.

Note also the distinction between closing the gap to MFEM and making MOOSE faster. They are not the
same list. Items 1 and 2 below are absolute improvements that would speed up MFEM too, so they do
nothing for the ratio while being the largest wins available.

Sized in wall time, not instruction counts. Two rankings in this file were wrong because an item's
share of instructions was used as a proxy for its share of time, and the two differ by more than a
factor of two here.

1. **Accept that roughly 0.45 s of the deficit is framework generality**, finding 19, and stop looking
   for a hidden item there. The residual after itemization is a long tail of sections each below 20 ms
   plus process startup, where MOOSE already beats MFEM. The only part of the remaining deficit with an
   identified mechanism is the 0.54 s of structural work in the assembly path, and `MatSetPreallocationCOO`
   at 0.253 s of that is irreducible on our side per finding 17. What is actually left to win is
   therefore small, and a session that wants a large number should change the benchmark rather than
   the code.

2. **Expand a node-level sparsity graph, or better, replace incremental insertion with a graph
   product**, finding 18 and the MFEM comparison below. Worth about 0.14 s best case, which is 2.3% of
   runtime: `compute_sparsity` is 4% of the run's instructions but only 0.16 s of its wall time. That
   sizing is the reason this is second rather than first. Of the two routes, the graph product that
   MFEM uses for scalar spaces, `Transpose(elem_dof, dof_elem)` then `Mult(dof_elem, elem_dof,
   dof_dof)` then `SortRows`, has no validity conditions to guard and helps every libMesh system,
   where the node-space expansion only helps multi-component variables and needs the fallback
   conditions enumerated in the design discussion. Note that MFEM does not do either for a vector
   space: `BilinearForm::AllocMat` bails out on `GetVDim() > 1` and assembles into dynamic rows, and
   `precompute_sparsity` defaults to 0 regardless. So this would be doing something MFEM does not,
   and MOOSE-Kokkos cannot copy MFEM here because the device CSR needs the pattern up front.

3. **Attribute and then reduce `Kokkos::Assembly::init`**, ~0.07 s of wall time, the largest
   MOOSE-owned setup item after the sparsity work, finding 18. Shape tables are per element type, so
   there is no obvious reason for it to be this large; per-element material property indexing is the
   untested suspicion.

3. **Out of scope, recorded for completeness: the OpenMP guard overhead inside hypre**, finding 10: 3.93 G instructions, 6.88% of everything
   executed inside `SNESSolve`, spent in libgomp's `omp_get_level` deciding whether a loop is already
   inside a parallel region, in a single-threaded run. It is the cleanest identified inefficiency in
   the hottest code on either side, it is fixable in hypre or by building hypre without OpenMP, and it
   would benefit every MOOSE and MFEM user. Finding 10 set it aside as "not a parity lever", which was
   true and beside the point.

   Confirm in time before acting: 6.88% is an instruction share, and `omp_get_level` is cheap per
   call, so the wall-clock share could be smaller. The cheap test is a hypre built without OpenMP.

   For scale, the same profile puts 34% of solve instructions in
   `hypre_BoomerAMGRelaxHybridGaussSeidel_core` and 14% in `hypre_CSRMatrixMatvecOutOfPlaceHost`, so
   roughly half of the solve is two hypre kernels. Any improvement there dominates everything else in
   this file.

4. **Out of scope: whether PETSc's host default of CF-relaxation is right**, finding 15. Answering it
   means surveying the problems MOOSE users actually run rather than this input pair, so it is not
   being pursued here. On this operator it costs
   23% of every preconditioner application and buys zero iterations. PETSc enables it deliberately, so
   the question is which problems it earns its keep on; if the answer is "few of the ones MOOSE users
   run", the default is worth revisiting upstream rather than overridden input by input. Needs evidence
   from problems other than this one.

3. **Setup, 0.44 s**, the largest remaining MOOSE-owned parity item. `FEProblem::InitializeKokkos` at
   0.22 s has no MFEM analogue at all and `FEProblem::EquationSystems::Init` is another 0.22 s. No
   mechanism identified yet for either.

4. **A block COO path, if it can be made to reach hypre.** Finding 17 shows the preallocation cost is
   O(point entries) and that a block description would cut it and the index memory about ninefold.
   With upstream in scope this is no longer simply blocked, but it needs block storage end to end:
   PETSc would need COO for BAIJ, and hypre's ParCSR is point-wise, so the harder half is hypre.
   `hypre_ParCSRBlockMatrix` exists for nodal systems and would be the place to look.
2. **Symmetric elimination of essential constraints**, findings 6 and 16, now the last known
   solver-side difference between the two backends and the reason their hierarchies still diverge
   slightly. Not a performance target, since the libMesh solve is already the faster of the two, but
   `-ksp_type cg` is currently applied to a non-symmetric matrix. Finding 5 notes that the Kokkos
   value array makes this cheaper to implement than the traditional path would. Testing
   `automatic_scaling` as a partial remedy is blocked by the bug below.
3. **Setup, 0.46 s**, partly structural. Of libMesh's 1.16 s, `FEProblem::InitializeKokkos` is
   0.22 s with no MFEM analogue and `FEProblem::EquationSystems::Init` is 0.22 s, against MFEM's
   0.68 s of mesh construction and essentially nothing else. Mesh generation on the libMesh side
   is already faster than MFEM's. Note that `KokkosSystem::MatSetPreallocation` is *not* part of
   setup, contrary to what an earlier version of this file said; it runs in execute, inside the
   first Jacobian evaluation.
4. **Symmetric constraint elimination**, finding 6. Its cost has never been isolated, and the
   Kokkos value array makes it cheaper to implement than it would be on the traditional path
   (finding 5). Worth doing for the symmetry of the operator regardless of the timing.
5. **A block formulation for vector element kernels. Not a priority.** The residual assembly
   difference is structural, per finding 11, and is worth under 0.1 s on this benchmark, so this is
   recorded as a direction rather than as work to schedule. It would mean a hook that returns the
   constitutive tensor per quadrature point and lets the framework do the block algebra, the way
   MFEM factors `pelmat` out across components and treats the lambda term as a rank-one update,
   instead of the present contract where the hook is handed a single trial index and must return a
   tensor for it. That is a framework interface change affecting every vector kernel and the AD
   paths, and it should be justified by a problem where assembly dominates rather than by this one.
   Smaller and unrelated: `VectorKernelValue` and the face gradient tables used by
   `VectorIntegratedBC` never received the finding 4a treatment.

One smaller loose end carried from earlier: `KokkosVectorNeumannBC` still has no documentation page,
which it needs before a pull request. `KokkosLinearElasticity` is gone, replaced by the production
objects of finding 22, which carry pages. Also `find_global_indices`,
0.579 s in the real partitioning, was flagged early and never revisited, though it matters less now
that the fast configuration never partitions a replicated mesh.

## Open bugs found along the way

Both should be filed as their own issues rather than folded into performance work.

### Any AuxKernel makes a distributed mesh hold every element on every rank

`AuxKernelBase::validParams()` registers a `GhostLowerDElems` relationship manager as
`GEOMETRIC | ALGEBRAIC`. Because it is geometric and cannot be attached early,
`MooseApp::attachRelationshipManagers` calls `MooseMesh::allowRemoteElementRemoval(false)`, so a
`DistributedMesh` retains every element on every rank through generation, refinement and
preparation. The functor itself is free, short-circuiting on `!hasLowerD()`; only the declaration
costs. Measured with `--mesh-only`, so no variables or kernels exist at all, on 262,144 elements
with `Mesh/uniform_refine=3`: 3.2 s against 7.2 s at one rank, 15.2 against 6.2 at four, 27.2
against 9.7 at 24. Any aux kernel does it, including `ConstantAux`, which couples nothing.

Two fixes were tried and both fail. Declaring it `ALGEBRAIC` only removes the cost entirely and the
`lower_d_var` tests still pass to eight ranks distributed, but it deletes a safeguard added
deliberately for parallel failures in #26593, and libMesh's documentation warns that once evaluable
elements are lost they cannot be recovered. Conditioning on `mesh.hasLowerD()` is impossible at the
binding decision point: that is `attachRelationshipManagers(MeshBase &, MooseMesh &)` called from
`MooseMesh::buildTypedMesh`, when the `MooseMesh` has no `MeshBase` attached, and calling
`hasLowerD()` there segfaults. Adding the check only at the later `attach_geometric_rm` task builds
and runs but changes nothing, because the flag was already cleared at construction. A real fix must
let the decision be revisited once the mesh exists, and `_allow_remote_element_removal` is a single
bool with several independent writers (`CreateDisplacedProblemAction`, `AddPeriodicBCAction`,
`Adaptivity`, `RefineSidesetGenerator`), so re-enabling it blindly would clobber them. This wants a
design decision about tracking why remote elements are being retained.

### automatic_scaling segfaults on any Kokkos kernel

Pre-existing and reproducible on the framework's own
`test/tests/kokkos/coord_type/kokkos_diffusion_rspherical.i` with
`Executioner/automatic_scaling=true`. The traditional path is fine. The crash is in
`Moose::Kokkos::Matrix::create`, reached from `System::sync` and `computeKokkosJacobian` inside
`NonlinearSystemBase::computeScaling()` during `preSolve()`, where
`_nr = sparsity.row_ptr.size() - 1` underflows on a sparsity that has not been built yet. This
blocks testing automatic scaling as a remedy for the constraint scaling mismatch in finding 6.

## Session log

- **2026-09-19 and earlier.** Gap 2.04x (12.66 s against 6.20 s), both sides without interpolation
  truncation or aggressive coarsening. Node-to-block relation storage rewritten in
  `MooseMesh::cacheInfo()`, taking it from 0.517 s to 0.041 s and removing roughly 140 MB per rank.
  `GeneratedMeshGenerator` partition-discard fix. Face-scoped AD accessors, `KokkosLinearElasticity`,
  `KokkosVectorNeumannBC`.
- **2026-09-20.** AMG configuration matched on both sides; benchmark inputs committed into this
  directory under a spec named `performance` so the default `run_tests` does not collect them;
  `MFEMHypreBoomerAMG` given a `vector_treatment` parameter. First handoff written, then found to
  have been measured against a stale mixed binary and superseded. Gap established at 1.44x.
- **2026-09-22 (later).** Found that PETSc's CF-relaxation, which MFEM never enables, was the whole
  of the preconditioner application gap: `-pc_hypre_boomeramg_no_CF` takes the run from 6.81 s to
  6.18 s at unchanged iteration count, putting the linear solve ahead of MFEM's and the gap at 1.23x
  (finding 15). Committed as "Match MFEM's relaxation ordering in the cantilever comparison".

- **2026-09-22.** Recorded the MFEM assembly level and the structural difference between the two
  element kernels (findings 11 and 12); read PETSc's COO implementation from the submodule and
  withdrew the recommendation to replace it with CSR (findings 9 and 13); allocated the Kokkos matrix
  values after the COO preallocation. Then found that the "unaccounted" 0.33 s was `PCSetUp` all
  along, which withdrew the old finding 8 and also corrected the per-iteration comparison from 1.03x
  to 1.18x (finding 14), leaving `PCApply` as the largest open item. Committed as three changes, by
  subject: contracting one row of a vector shape gradient, allocating the Kokkos matrix values after
  the COO preallocation, and assembling the cantilever Jacobian directly as a hypre matrix. Verified
  against 254 passing Kokkos and vector-FE tests and the three performance tests.

- **2026-09-21.** Re-measured on a consistent build after repairing the stale libtool archives.
  Added `-nl0_mat_type hypre` to `kokkos_cantilever.i`, taking the gap from 1.43x to 1.39x. Refuted
  the preconditioner-plumbing hypothesis and the 0.88 s solve deficit. Then condensed the vector
  shape gradient table, after first trying and measuring a component-only form that was a
  regression, taking assembly from 1.139 s to 0.834 s and the gap to 1.35x. The solver is now
  within 5% of MFEM per iteration and assembly is near parity, which moves the non-kernel execute
  work to the top of the list.

#### Device memory is not in the perf graph

The perf graph's `Mem(MB)` column is host resident memory. Kokkos material property storage is
allocated on the device through `createDevice()`, so none of it appears there. Reverting the tangent of
`KokkosComputeLinearElasticStress` to per-quadrature-point storage and back changed the reported figure
from 4615 MB to 4619 MB, which is noise, while the device footprint moved by 1.3 GB. Any claim about
Kokkos property memory has to come from a device measurement.

`/tmp/peak_gpu.sh` in that session sampled `nvidia-smi --query-compute-apps=pid,used_gpu_memory`
filtered to the run's own PID, every 0.2 s, and reported the maximum. Sampling is coarse enough to miss
a short-lived spike, which is acceptable for a footprint dominated by a persistent allocation.

#### Per-property storage granularity, measured

MOOSE `49df91c3b7a` plus the #33855 work, apptainer
`moose-dev-ubuntu24-cuda-gcc-openmpi-x86_64_v3:2026.08.23`, libMesh from `libmesh/installed`,
`METHOD=opt`, one NVIDIA L4. `kokkos_cantilever.i` at `refinement = 5`, 262144 elements, 839619 DoFs.

| tangent storage | peak device memory |
| --------------- | ------------------ |
| per quadrature point | 13410 MiB |
| per subdomain | 12104 MiB |

The difference is 1306 MiB against a predicted 262144 x 8 x 648 B = 1296 MiB, so the saving is the
whole of `Jacobian_mult` to within 1 percent. `maxZ` is unchanged to the test's tolerance; the last two
digits move, which is the atomic-add assembly's run-to-run reproducibility and not a consequence of the
change.

Two traps hit while measuring. This container's hypre is built GPU-aware and rejects host vectors, so
the cantilever needs `-vec_type cuda` on any device, including `cpu`; without it the run dies in
`PCApply_HYPRE` and the performance tests fail for reasons unrelated to whatever is being measured.
And the build targets compute capability 8.0 while the L4s are 8.9, which Kokkos warns about at
startup, so wall times from this machine are indicative only.
