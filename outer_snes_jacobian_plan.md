# Outer SNES Full Jacobian Assembly

## Context

The nonlinear preconditioning (NPC) implementation uses a block-diagonal outer SNES Jacobian
(J₁₁, J₂₂ on the diagonal; J₁₂ = J₂₁ = 0). This gives **linear** outer convergence. To
achieve **quadratic** outer convergence the outer SNES Jacobian must include the off-diagonal
blocks J₁₂ and J₂₁.

The required outer Jacobian depends on the inner solve strategy:

**Pure nonlinear elimination (inner system converged):** the inner solve eliminates x₂ in terms
of x₁. The reduced problem for x₁ has the Schur complement as its exact Jacobian:

```
J_red = J₁₁ − J₁₂ · J₂₂⁻¹ · J₂₁
```

**Nonlinear Gauss-Seidel / approximate inner solve:** the inner system is NOT fully converged;
x₂ is not slaved to x₁. The outer Newton step acts on both x₁ and x₂ together. The full
block-structured Jacobian is needed:

```
J = [ J₁₁  J₁₂ ]
    [ J₂₁  J₂₂ ]
```

In both cases assembling J₁₂ and J₂₁ is the prerequisite. The outer KSP preconditioner type
then determines how the block structure is exploited — this is a solver choice, not an
assembly choice.

---

## Why Cross-System Jacobian Assembly Is Non-Trivial

### The AD seeding collision problem

In MOOSE's AD framework, dual-number derivative slots are indexed by the DOF numbers of the
**seeded system**. When a kernel in system `i` is assembled, system `i`'s DOFs are seeded as
the dual-number inputs. A variable coupled from system `j` contributes to the residual value
but its DOFs are unseeded — so the AD-computed Jacobian returns ∂F_i/∂x_j = 0.

Naively seeding both systems simultaneously causes a collision: systems `i` and `j` may have
overlapping local DOF numberings (both start at 0 within their own system), making it
impossible to distinguish derivative slots.

### The handwritten-Jacobian case

For non-AD kernels, `computeQpOffDiagJacobian(jvar)` is already called for coupled variables,
but `jvar` is expected to be a variable index within the current system's DOF space. When
`jvar` belongs to a different libMesh system, `Assembly::addJacobianBlock` calls
`_sys.getVariable(tid, jvar)` which errors because `jvar` is not registered in the current
system.

---

## The ISys / JSys Context Approach

Rather than creating per-(i,j) kernel copies, the same kernel is re-run with a different
**Jacobian block context** specifying the row system (ISys, whose residual is assembled) and
the column system (JSys, whose DOFs are seeded for AD).

### The `doDerivatives` change

`ADUtils.C` currently gates AD seeding on:

```cpp
bool Moose::doDerivatives(const SubProblem & subproblem, const SystemBase & sys)
{
  return ADReal::do_derivatives && sys.number() == subproblem.currentNlSysNum();
}
```

Change the check to the column system:

```cpp
  return ADReal::do_derivatives && sys.number() == subproblem.currentNlJSysNum();
```

Two new SubProblem accessors:

| Accessor | Meaning |
|---|---|
| `currentNlISysNum()` | Row system — whose residual F_i is being assembled |
| `currentNlJSysNum()` | Column system — whose DOFs are seeded for AD |

For all existing diagonal assembly (ISys == JSys): both equal `currentNlSysNum()`, no
behavioral change.

### How J_ij falls out automatically

When assembling block (i, j) with ISys = i, JSys = j:

- `doDerivatives(subproblem, sys_i)` → false — system i's own DOFs carry no derivatives
- `doDerivatives(subproblem, sys_j)` → true — system j's DOFs (read as coupled variables
  inside the kernel) are seeded

The kernel's `computeQpResidual()` runs unchanged. AD computes ∂F_i/∂x_j through the coupled
variable reads. No kernel copies, no new kernel code.

### Assembly loop for the full outer Jacobian

```
for each system pair (i, j):
  setISys(i), setJSys(j)
  for each kernel K in system i:
    re-run K's assembly with (ISys=i, JSys=j) context
    → contributions written to the J_ij matrix via the active tag for that block
```

For diagonal blocks (i == j) this reproduces existing behavior. For off-diagonal blocks
(i ≠ j) the same kernel loop is re-run with a different seeding context.

Note: even when the inner SNES for system i has just run, its diagonal block J_ii must still
be assembled explicitly in the outer Jacobian callback. The inner SNES does not necessarily
evaluate the Jacobian at the final (converged) iterate — the last inner iteration computes
the residual to check convergence, not the Jacobian.

---

## Off-Diagonal Matrix Storage

libMesh `System::matrix()` is always `n_dofs_i × n_dofs_i` (square). The cross-system block
J_ij is `n_dofs_i × n_dofs_j` and cannot be stored in either system's existing matrix.

Off-diagonal blocks are stored as standalone PETSc `Mat` objects owned by the
`NonlinearElimination` sub-block object. They are assembled into the **MatNest outer Jacobian**
(see next section).

### Reusing the matrix tag system

Rather than adding special-case routing in Assembly, off-diagonal mats slot into the existing
matrix tag infrastructure:

1. At setup, for each (i, j) pair with declared coupling, call
   `SubProblem::addMatrixTag("NPC_J_i_j")` → returns a unique `TagID`.
2. Allocate J_ij as a PETSc `Mat`; wrap it in a `PetscMatrix<Number>`.
3. Register it with system i via `sys_i.associateMatrixToTag(J_ij_wrapper, tag_id)`.
4. During off-diagonal assembly, activate only the `NPC_J_i_j` tag. The existing
   `Assembly::addJacobian` path writes to `_sys._tagged_matrices[tag_id]` = J_ij.

The ISys/JSys context handles the column DOF index routing (sys_j's DofMap for columns);
the tag system handles which matrix receives the contribution. The two mechanisms are
orthogonal and compose cleanly.

Sparsity for J_ij is determined once at setup by scanning kernels in system i for coupled
variables from system j, collecting the combined DOF overlap pattern from both DofMaps.

---

## Outer SNES with MatNest Jacobian

The outer SNES Jacobian is a PETSc `MatNest` with one sub-block per (i, j) system pair:

```
MatNest =  [ J₁₁   J₁₂  ...  ]
           [ J₂₁   J₂₂  ...  ]
           [  ...         ... ]
```

Diagonal blocks J_ii are the existing per-system libMesh matrices (reused, not copied).
Off-diagonal blocks J_ij are the standalone Mats described above.

The outer KSP sees this MatNest as its operator. PETSc's `pc_type fieldsplit` works natively
with MatNest and can apply any split strategy the user selects.

### Alternative: monolithic AIJ

Assemble all blocks into a single AIJ matrix with a combined global DOF numbering. This works
with `-pc_type lu` and other direct solvers without any block-structure configuration.
Deferred as a future path; the MatNest approach is implemented first.

---

## Outer KSP Preconditioner: Don't Hard-Code the Split Type

The appropriate fieldsplit type depends on which systems have inner SNESes and how tightly
they are converged. Any subset of systems may have inner SNESes; for those that do not, the
outer Newton step is the only update for that block.

| Configuration | Outer KSP PC | Notes |
|---|---|---|
| One inner SNES solved tightly/eliminated | `fieldsplit_type schur` | Eliminated system outer Newton update tied to non-eliminated system's |
| Any configuration | `fieldsplit_type multiplicative` | Block Gauss-Seidel at linear level |
| Any configuration | `fieldsplit_type symmetric_multiplicative` | Forward + backward sweep |
| Any configuration | `fieldsplit_type additive` | Independent block updates |

The implementation must **not** hard-code the fieldsplit type. The outer SNES prefix
(`foo_subblock_` in the example below) exposes the full PETSc options hierarchy to the user:

```
-foo_subblock_pc_type fieldsplit
-foo_subblock_pc_fieldsplit_type multiplicative
-foo_subblock_pc_fieldsplit_0_ksp_type preonly
-foo_subblock_pc_fieldsplit_0_pc_type lu
```

---

## Input Syntax

The top-level block is `[NonlinearElimination]` with named sub-blocks. Each sub-block creates
one new outer SNES; the sub-block name is the PETSc prefix for that SNES. `inner_nl_sys_names`
lists **all** systems that participate as sub-SNESes in the outer SNES's NPC composite:

- **Eliminated systems (NLE):** given a real SNES that solves the system.
- **Non-eliminated systems (NLE):** given a no-op SNESSHELL (identity); the outer Newton
  step is their only update.
- **All systems (NGS):** all given real SNESes.

The NPC composite type (additive, multiplicative, etc.) is set by the user via PETSc options
on the NPC's prefix — it is not hard-coded. PETSc options inside the sub-block are
automatically prefixed with the sub-block name; the user does not write the prefix manually.

```ini
[NonlinearElimination]
  [foo_subblock]
    inner_nl_sys_names = 'mechanics energy'  # all systems under this outer SNES
    petsc_options_iname = '-snes_type -snes_rtol'
    petsc_options_value  = 'newtonls 1e-8'
  []
[]
```

For pure NLE (eliminate energy, outer Newton on mechanics):
- `mechanics` sub-SNES: no-op SNESSHELL (identity)
- `energy` sub-SNES: real SNES configured via `[Preconditioning]` or PETSc options on its prefix

For NGS (all systems get inner sweeps before outer Newton):
- Both `mechanics` and `energy` sub-SNESes: real SNESes

---

## Implementation Plan

### Step 1: Fix handwritten-Jacobian cross-system lookup

In `Assembly::addJacobianBlock` (and `Assembly::addJacobianBlockTags`):

- When ISys ≠ JSys, look up the column variable in system j's DofMap.
- Accept cross-system coupling in the coupling matrix check.
- Route contributions to the (i, j) off-diagonal Mat via its tag (rather than the diagonal
  system matrix).

### Step 2: ISys / JSys context on SubProblem

Add `currentNlISysNum()` / `currentNlJSysNum()` accessors and a setter:

```cpp
void setJacobianBlockContext(unsigned int i_sys, unsigned int j_sys);
unsigned int currentNlISysNum() const;
unsigned int currentNlJSysNum() const;
```

For all existing code paths ISys == JSys == `currentNlSysNum()` — no behavioral change.

Change `Moose::doDerivatives` to check `sys.number() == subproblem.currentNlJSysNum()`.

### Step 3: Off-diagonal Mat allocation and tag registration

In the `NonlinearElimination` sub-block's `initialSetup()`:

- Scan each kernel in system i for coupled variables from system j.
- For each (i, j) pair with coupling:
  - Call `_fe_problem.addMatrixTag("NPC_J_i_j")` → `TagID`.
  - Allocate a `n_dofs_i × n_dofs_j` PETSc Mat with the correct sparsity.
  - Wrap in `PetscMatrix<Number>`; register via `sys_i.associateMatrixToTag(wrapper, tag_id)`.
  - Store the wrapper in `_off_diag_mats[{i,j}]`.

### Step 4: Outer SNES MatNest Jacobian callback

```
assembleOuterJacobian(x):
  for each system i:
    setJacobianBlockContext(i, i)
    activate system i's Jacobian tag
    computeJacobianSys(sys_i, x_i)         → fills J_ii diagonal block

  for each (i, j) pair with coupling, i ≠ j:
    setJacobianBlockContext(i, j)
    activate only the NPC_J_i_j tag
    re-run kernel assembly for system i    → fills J_ij off-diagonal block

  assemble MatNest from {J_ii} diagonal blocks + {J_ij} off-diagonal blocks
```

### Step 5: IS registration for fieldsplit

`MatNest` stores the IS for each block internally. After constructing the MatNest, retrieve
them via `MatNestGetISs` and register with the outer PC:

```cpp
IS * row_is;
MatNestGetISs(mat_nest, &row_is, NULL);
for each system i:
  PCFieldSplitSetIS(pc, sys_i.name(), row_is[i]);
```

This requires no global DOF numbering beyond what the MatNest already maintains — the IS for
each block is a contiguous range in the combined nest vector and is computed automatically
when the MatNest is created.

---

## Files to Create / Modify

| File | Change |
|---|---|
| `framework/src/utils/ADUtils.C` | Step 2: `doDerivatives` checks `currentNlJSysNum()` |
| `framework/include/problems/SubProblem.h` | Step 2: ISys/JSys accessors and setter |
| `framework/src/problems/SubProblem.C` | Step 2: ISys/JSys state |
| `framework/src/base/Assembly.C` | Step 1: cross-system jvar lookup and tag-based routing |
| `framework/include/base/Assembly.h` | Step 1 |
| `framework/src/preconditioners/NonlinearPreconditioning.C` | Steps 3–5; rename to NonlinearElimination |
| `framework/include/preconditioners/NonlinearPreconditioning.h` | Steps 3–5; rename |
| `framework/src/actions/SetupNonlinearPreconditioningAction.C` | Sub-block syntax; rename |
| `framework/src/base/Moose.C` | Register `NonlinearElimination/*/` syntax |
