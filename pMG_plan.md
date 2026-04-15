# Plan: P-Multigrid Preconditioner (Issue #32771)

## Background

Multigrid is the gold standard for scalable iterative solvers, but algebraic multigrid
can run out of memory easily for large device calculations. The scientific community has
moved toward p-multigrid with partial assembly on fine levels to get full value from
hardware accelerators. MFEM already provides `GeometricMultigrid` (in
`contrib/mfem/fem/multigrid.hpp`) and `ParFiniteElementSpaceHierarchy` (in
`contrib/mfem/fem/fespacehierarchy.hpp`). MOOSE needs a wrapper that plugs into the
existing `LinearSolverBase` / `Preconditioner` block infrastructure.

The interface must **not** be tied to `mfem::ParBilinearForm`. As demonstrated in
`framework/contrib/mfem/miniapps/neml2/neml2.cpp`, geometric multigrid works entirely
at the `mfem::Operator` level: each level holds a form (bilinear or nonlinear) for
lifetime management, and the operator is obtained form-agnostically —
`GetGradient(x)` for nonlinear forms, the assembled matrix for bilinear forms.

The smoother at each level is a user-specified `LinearSolverBase` object. Simple
smoothers (Chebyshev, Jacobi) only need `opr.AssembleDiagonal(diag)`, but
factorization-based smoothers such as ILU require a fully assembled sparse matrix.
The smoother's own `updateSolver(op, tdofs)` encapsulates this — the MG preconditioner
just calls through to it. The assembly level at each level must therefore be compatible
with the smoother chosen for that level: `PARTIAL` is sufficient for diagonal-based
smoothers, but `FULL` is required for ILU and similar methods.

Reference MFEM PR: https://github.com/mfem/mfem/pull/5142

---

## Step 1 — Generalize `LinearSolverBase::updateSolver`

**File:** `framework/include/mfem/solvers/MFEMLinearSolverBase.h`

Add a second virtual overload that works at the `mfem::Operator` level:

```cpp
virtual void updateSolver(mfem::Operator & op, mfem::Array<int> & tdofs);
```

Provide a default no-op implementation so existing subclasses are unaffected. The
existing `updateSolver(mfem::ParBilinearForm &, mfem::Array<int> &)` overload stays
for solvers like `Moose::MFEM::HypreBoomerAMG` that require the assembled matrix.
`Moose::MFEM::GeometricMultigridSolver` overrides the `Operator &` version only.

---

## Step 2 — Form factory methods on `EquationSystem`

**Files:** `framework/include/mfem/equation_systems/EquationSystem.h` / `.C`

`EquationSystem` already has everything needed: `_kernels_map` (all kernels indexed by
test/trial variable), `_non_linear` (flag indicating nonlinear integrators are
present), and the existing `ApplyDomainBLFIntegrators` / `ApplyDomainNLFIntegrators`
template helpers. Two new factory methods are added — the multigrid solver calls them
and **owns** the returned forms:

```cpp
/// Build a fresh ParBilinearForm on the given FESpace using the same kernels as
/// the main system's bilinear form for var_name. Caller owns the returned form.
std::shared_ptr<mfem::ParBilinearForm>
buildBilinearFormForFESpace(const std::string & var_name,
                            mfem::ParFiniteElementSpace & fespace,
                            mfem::AssemblyLevel assembly_level);

/// Build a fresh ParNonlinearForm on the given FESpace using the same kernels as
/// the main system's nonlinear form for var_name. Caller owns the returned form.
std::shared_ptr<mfem::ParNonlinearForm>
buildNonlinearFormForFESpace(const std::string & var_name,
                             mfem::ParFiniteElementSpace & fespace,
                             mfem::AssemblyLevel assembly_level);
```

Both methods iterate `_kernels_map` for `var_name` and call the existing
`ApplyDomainBLFIntegrators` / `ApplyDomainNLFIntegrators` helpers with the new
`fespace`-backed form. The `_non_linear` flag on `EquationSystem` tells the MG
solver which method to call (or both, if mixed linear/nonlinear integrators are
present on the same variable).

The MG solver stores the returned forms in its own members (see Step 3). Lifetime is
managed there, not in `EquationSystem`.

---

## Step 3 — Consolidate `Preconditioner` block into `Solvers`

The `Preconditioner` block (handled by `AddMFEMPreconditionerAction`) and the
`Solvers` block (handled by `AddMFEMSolverAction`) register objects in exactly the
same way — both call `addObject<SolverBase>` via the MFEM warehouse. The only
difference is that
`addMFEMSolver` also writes objects into `problem_data.jacobian_solver` or
`problem_data.nonlinear_solver` based on type. The `Preconditioner` block exists
purely as a workaround to avoid clobbering those fields with auxiliary solvers.

The relationship between the nonlinear and linear solver is currently implicit
(problem data holds both and the executioner uses them together). The cleaner design
makes it explicit: `Moose::MFEM::NewtonNonlinearSolver` owns its linear solver via a
`linear_solver` parameter, removing the need for `jacobian_solver` in
`Moose::MFEM::ProblemData` entirely. For pure linear problems, the outer solver is the one not
referenced as `preconditioner` or `linear_solver` by any other solver in the block —
determinable at setup time by scanning all solver objects in the warehouse.

**Proposed input file for nonlinear problem:**

```
[Solvers]
  [boomeramg]
    type = MFEMHypreBoomerAMG
  []
  [lin]
    type = MFEMHypreGMRES
    preconditioner = boomeramg
  []
  [newton]
    type = MFEMNewtonNonlinearSolver
    linear_solver = lin
  []
[]
```

**Proposed input file for linear problem:**

```
[Solvers]
  [boomeramg]
    type = MFEMHypreBoomerAMG
  []
  [main]
    type = MFEMHypreGMRES
    preconditioner = boomeramg
  []
[]
# outer solver inferred: 'main' is not referenced by any other solver
```

**Implementation changes:**
- Add `linear_solver` parameter to `Moose::MFEM::NewtonNonlinearSolver` (and
  `Moose::MFEM::PetscNonlinearSolver`). The nonlinear solver resolves and owns the
  linear solver at construction time.
- `addMFEMSolver` becomes identical to `addMFEMPreconditioner` — just
  `addObject<SolverBase>`, no side-effects on problem data. At setup time, the
  problem identifies the outer solver by elimination (the unreferenced one) and
  stores it.
- Remove `jacobian_solver` from `Moose::MFEM::ProblemData`; only `nonlinear_solver` remains
  (or also goes away if the nonlinear solver fully owns the linear one).
- `AddMFEMPreconditionerAction` is deleted; its task re-points at
  `AddMFEMSolverAction`, and the `Preconditioner` block emits a deprecation error
  directing users to `Solvers`.
- Update all existing test input files.

---

## Step 4 — `Moose::MFEM::GeometricMultigridSolver` class

**New files:**
- `framework/include/mfem/solvers/MFEMGeometricMultigridSolver.h`
- `framework/src/mfem/solvers/MFEMGeometricMultigridSolver.C`

Inherits from `Moose::MFEM::LinearSolverBase`. Wraps `mfem::GeometricMultigrid`.

`GeometricMultigrid` takes a `ParFiniteElementSpaceHierarchy` as a constructor
argument — the hierarchy is an independent object that may represent h-refinement,
p-refinement, or a combination. The solver accepts a reference to a new
`Moose::MFEM::FESpaceHierarchy` object (see Step 4a). The number of levels `N` comes from
that object and drives all vector length consistency checks.

With the consolidated `Solvers` block the input file looks like:

```
[FESpaceHierarchies]
  [h1_hierarchy]
    type = MFEMFESpaceHierarchy
    fespace = H1FESpace
    refinements = '1 2 4'  # p-refine to orders 1, 2, 4 — or mix with 'h' entries
  []
[]

[Solvers]
  [boomeramg]
    type = MFEMHypreBoomerAMG
  []
  [cheby]
    type = MFEMOperatorChebyshevSmoother  # only needs AssembleDiagonal — PARTIAL is fine
  []
  [ilu_smoother]
    type = MFEMBlockILUSmoother           # needs a sparse matrix — requires FULL at that level
  []
  [pmg]
    type = MFEMGeometricMultigridSolver
    variable = u
    fespace_hierarchy = h1_hierarchy      # N levels; drives consistency checks
    smoothers = 'ilu_smoother cheby'      # length 1 or N-1, coarse-to-fine interior levels
    coarse_solver = boomeramg
    assembly_levels = 'full full partial' # length 1 or N, coarse-to-fine all levels
  []
  [main]
    type = MFEMCGSolver
    preconditioner = pmg
  []
[]
```

**Vector length consistency checks** (performed at setup time once `N =
hierarchy.GetNumLevels()` is known):

- `smoothers`: length must be 1 or `N - 1`; a `mooseError` otherwise.
- `assembly_levels`: length must be 1 or `N`; a `mooseError` otherwise.
- Length-1 entries are broadcast to all applicable levels.
- Per-level smoother–assembly-level compatibility is checked after broadcasting
  (e.g. ILU smoother with `partial` assembly emits a `mooseError`).

---

## Step 4a — `Moose::MFEM::FESpaceHierarchy` (prerequisite)

**New files:**
- `framework/include/mfem/fespaces/MFEMFESpaceHierarchy.h`
- `framework/src/mfem/fespaces/MFEMFESpaceHierarchy.C`

A `Moose::MFEM::Object` that owns a
`mfem::ParFiniteElementSpaceHierarchy` and exposes it via `getHierarchy()`. It takes
a base `Moose::MFEM::FESpace` (the coarsest level) and a `refinements` vector of mixed entries
encoding the construction sequence:

- **`h`** — calls `AddUniformlyRefinedLevel()` (same FEC, refined mesh)
- **integer** — calls `AddOrderRefinedLevel()` with a new FEC of that order

```
[FESpaceHierarchies]
  [h1_hierarchy]
    type = MFEMFESpaceHierarchy
    fespace = H1FESpace          # coarsest level
    refinements = 'h h 2 4'     # two h-refinements then p-refine to order 2 then 4
  []
[]
```

`N = refinements.size() + 1`. The FEC type for p-levels is inferred from the base
`Moose::MFEM::FESpace`. For p-entries, a new `FiniteElementCollection` of the specified order
is created and stored alongside the hierarchy (MFEM does not own them). Emits a
`mooseError` if a p-entry order is not strictly greater than the previous level's
order. This object is separate from `Moose::MFEM::GeometricMultigridSolver` so the same
hierarchy can be shared or inspected independently.

### Key members

```cpp
std::unique_ptr<mfem::GeometricMultigrid> _mg;
// Forms at each level — owned here, built via EquationSystem factory methods
std::vector<std::shared_ptr<mfem::ParBilinearForm>>  _level_blfs;
std::vector<std::shared_ptr<mfem::ParNonlinearForm>> _level_nlfs;
```

The `Moose::MFEM::FESpaceHierarchy` object owns the `ParFiniteElementSpaceHierarchy`
and must therefore outlive `_mg`. `Moose::MFEM::ProblemData` should hold a
`shared_ptr` to the hierarchy (analogous to how `fespaces` are stored), and
`Moose::MFEM::GeometricMultigridSolver` should co-own it via a `shared_ptr` member —
ensuring the hierarchy is kept alive at least as long as any solver that references it.

### `constructSolver()`

Builds a placeholder (empty `mfem::OperatorJacobiSmoother`). Real work is deferred
to `updateSolver`.

### `updateSolver(mfem::Operator & op, mfem::Array<int> & tdofs)`

Called once per linear/nonlinear solve. Steps:

1. Obtain `N = fespace_hierarchy.getHierarchy().GetNumLevels()` and validate
   `smoothers` and `assembly_levels` vector lengths (broadcast if length 1).
2. Pass `ess_bdr` boundary attribute array to the
   `GeometricMultigrid(fespace_hierarchy.getHierarchy(), ess_bdr)` constructor so
   MFEM computes essential dofs at each level automatically.
3. For each level `i`, call `eq_sys.buildBilinearFormForFESpace(var,
   hierarchy.GetFESpaceAtLevel(i), assembly_levels[i])` and/or
   `buildNonlinearFormForFESpace(...)` depending on `eq_sys.nonlinear()`. Store the
   returned forms in `_level_blfs` / `_level_nlfs`. Extract the operator: assembled
   matrix for bilinear forms; `GetGradient(level_solution)` for nonlinear forms (see
   open question 1 for the linearization point).
4. For each interior level, resolve the smoother from the broadcast/per-level
   `smoothers` vector and call `smoother.updateSolver(level_op, level_tdofs)`.
5. Attach the `coarse_solver` at level 0 via its `updateSolver`.
6. Populate the `GeometricMultigrid` by calling `AddLevel(op, smoother, ownOp,
   ownSmoother)` for each level, then set `_solver` to point to it.

---

## Step 5 — Registration and build

- `registerMooseObject("MooseApp", MFEMGeometricMultigridSolver)` and
  `registerMooseObject("MooseApp", MFEMFESpaceHierarchy)` in their respective `.C`
  files.
- Add all new `.C` files to `framework/CMakeLists.txt`.
- Add includes where other MFEM solver/fespace headers are gathered.

---

## Step 6 — Tests

Under `test/tests/mfem/solvers/`:

1. **Linear test:** Poisson with a high-order H1 space. Use
   `MFEMGeometricMultigridSolver` as preconditioner for `MFEMCGSolver` with
   `fine_assembly_level = partial` and `coarse_assembly_level = full`. Verify
   convergence.
2. **Nonlinear test:** Nonlinear diffusion or elasticity. Use
   `MFEMGeometricMultigridSolver` via the nonlinear solver path with PETSc. Verify
   that the preconditioner factory rebuilds the MG at each Newton step.

---

## Open questions

1. **Linearization point for nonlinear coarse levels.** `neml2.cpp` restricts the
   current fine solution to coarser levels via
   `prolongations[level]->MultTranspose(finer_solution, coarse_solution)` before
   calling `GetGradient()`. `Moose::MFEM::GeometricMultigridSolver` needs access to the
   current solution vector — likely threaded through `updateSolver` or obtained from
   `Moose::MFEM::Problem`.

2. **Operator ownership.** `AddLevel(op, smoother, ownOp, ownSmoother)` — pass
   `ownOp=false` when the operator is owned by a form living in `EquationSystem`.
   Ownership must be consistent to avoid double-free.

3. **`ParFiniteElementSpaceHierarchy` lifetime.** `GeometricMultigrid` stores a
   reference to the hierarchy owned by `Moose::MFEM::FESpaceHierarchy`. Lifetime is
   managed through `shared_ptr`: `Moose::MFEM::ProblemData` holds the hierarchy
   alongside other FE spaces, and `Moose::MFEM::GeometricMultigridSolver` co-owns it
   via a `shared_ptr` member,
   guaranteeing the hierarchy outlives any solver that references it.

4. **Mixed / complex equation systems.** Initial implementation targets
   `EquationSystem` only. `TimeDependentEquationSystem` and
   `ComplexEquationSystem` support can be added as follow-ons.

5. **Per-level smoother resolution.** The `smoothers` vector is resolved against
   warehouse objects at setup time. When a single name is given it is broadcast to all
   interior levels; the same `LinearSolverBase` instance is reused, so its
   `updateSolver` must be idempotent across repeated calls with different
   level operators.

6. **Smoother–assembly-level compatibility checking.** At setup time the solver
   should inspect the smoother type and verify the assembly level at that level is
   sufficient (e.g. `BlockILU` requires `FULL`; `OperatorChebyshevSmoother` works
   with `PARTIAL`). The mechanism for this — whether via a virtual method on
   `LinearSolverBase` or an explicit type check — is to be decided.

7. **Mixed bilinear forms not supported.** `mfem::GeometricMultigrid` takes a
   single `FiniteElementSpaceHierarchy`, so its prolongation operators and per-level
   `BilinearForm` array all operate on a single FESpace type. Saddle-point or
   mixed-field problems (e.g. H(div)–L2 Darcy, H(curl)–H1 magnetics) that use
   `_mblfs` off-diagonal blocks cannot be handled by this class as-is. Block
   multigrid with separate hierarchies per field would be needed and is out of scope
   for the initial implementation. `Moose::MFEM::GeometricMultigridSolver` should emit a
   `mooseError` if the target variable's equation system has active mixed bilinear
   form contributions.
