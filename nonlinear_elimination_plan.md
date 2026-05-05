# Nonlinear Elimination in MOOSE

## Motivation

Nonlinear elimination (Lanzkron, Rose & Wilkes, SISC 1996) solves subsystems of a nonlinear system as a nonlinear preconditioner before applying a global Newton update. The canonical application is charge transport (Scharfetter-Gummel iteration), but the technique is broadly useful for multiphysics systems where one set of fields dominates the Newton direction and prevents correct updates to other critical fields.

A key practical example: in thermofluid problems, the mechanical DOFs (velocity `u` and pressure `p`) dominate the Newton step, leaving temperature `T` nearly unchanged each iteration. But a globally correct `T` is necessary for overall convergence. Nonlinear elimination treats the temperature subsystem as a nonlinear preconditioner that is solved to convergence before the global Newton step is taken, breaking the coupling deadlock.

This can be thought of as **nonlinear FieldSplit** or **nonlinear PCPatch**.

---

## Mathematics

### The left-preconditioned nonlinear system

Linear left preconditioning solves P⁻¹(Ax − b) = 0. The nonlinear analog replaces the linear preconditioner P⁻¹ with a nonlinear operator M: ℝⁿ → ℝⁿ. Given current iterate x, M(x) is the result of applying the nonlinear preconditioner (e.g., one sweep of sequential physics solves). The left-preconditioned system is:

> G(x) = F(M(x)) = 0

Newton on G yields the effective Jacobian:

> G′(x) δx = −G(x)
> [J(M(x)) · M′(x)] δx = −F(M(x))

The effective Jacobian is **J(M(x)) · M′(x)** — the original Jacobian evaluated at the preconditioned point, times the Jacobian of M itself.

### What happens in practice

Computing M′(x) requires differentiating through the inner nonlinear solve, which is intractable in general. PETSc drops this term. The outer Newton step at iterate xᵏ is:

1. Apply M: **w = M(xᵏ)** — the sequential inner SNES solves
2. Evaluate: **F(w)** and **J(w)**
3. Solve: **J(w) δ = −F(w)**
4. Update: **xᵏ⁺¹ = w + α δ** (with line search)

The outer solver uses J(w) alone — the Jacobian at the preconditioned point — rather than the full J(M(x))·M′(x). This is an approximation to Newton on G, but an effective one: w is already a much better iterate because the inner solves have already satisfied the subsystem residuals.

### Connection to the Schur complement

For pure nonlinear elimination — solving subsystem 2 exactly for x₂ given x₁ — M has a specific structure. Partition x = (x₁, x₂), F = (F₁, F₂):

- M maps (x₁, x₂) → (x₁, g(x₁)) where g(x₁) solves F₂(x₁, x₂) = 0
- The reduced problem is F₁(x₁, g(x₁)) = 0
- Its Jacobian, obtained by differentiating F₂(x₁, g(x₁)) = 0 for dg/dx₁, is:

> **J_red = J₁₁ − J₁₂ · J₂₂⁻¹ · J₂₁**

This is the **Schur complement** of J — the same operator that linear FieldSplit with a Schur factorization computes, but now arising from a nonlinear elimination rather than a linear block factorization. This is the precise sense in which nonlinear elimination is "nonlinear FieldSplit."

### Block-diagonal vs. full outer Jacobian

The outer Newton step uses J(w) assembled at the preconditioned point w = M(xᵏ). For a two-system partition, J(w) has the block structure:

```
J(w) = [ J₁₁(w)   J₁₂(w) ]
       [ J₂₁(w)   J₂₂(w) ]
```

**Block-diagonal approximation** (drop J₁₂, J₂₁):

```
J_diag(w) = [ J₁₁(w)    0    ]
            [    0    J₂₂(w) ]
```

This decouples the outer Newton correction into two independent linear solves — δx₁ = −J₁₁⁻¹F₁(w), δx₂ = −J₂₂⁻¹F₂(w) — which is additive at the linear correction level. The combined algorithm is therefore **multiplicative at the nonlinear level** (the NPC applies inner solves sequentially) and **additive at the outer linear correction level** (each subsystem's correction is computed ignoring the other). This is distinct from additive SNESCOMPOSITE, which is additive at both levels.

Compared to pure nonlinear elimination (inner solve 2 fully converged), the block-diagonal outer Jacobian is actually weaker: exact NLE gives the Schur complement J₁₁ − J₁₂·J₂₂⁻¹·J₂₁ as the outer Jacobian for x₁ for free, while the block-diagonal drops the correction term J₁₂·J₂₂⁻¹·J₂₁. When the NPC is effective (w near the solution), J₁₂(w) and J₂₁(w) are small and the approximation is adequate. For strong cross-system coupling with loose inner tolerances, adding the off-diagonal blocks recovers the Schur complement quality.

### Convergence: asymptotic rate vs. path to the quadratic basin

Near the root x*, write xᵏ = x* + eᵏ. One inner Newton step on x₂ (with x₁ frozen) gives:

> x₂^new = x₂* − J₂₂⁻¹J₂₁ e₁ᵏ + O(‖eᵏ‖²)

This is identical to the leading term of the exact implicit function g(x₁ᵏ) from the implicit function theorem — the two agree to O(‖eᵏ‖²). The outer Newton step with the exact Schur complement therefore still achieves quadratic convergence: full convergence of x₂ is not required asymptotically.

The key is that "loose" inner solves means "few inner Newton iterations," not "non-Newton relaxation." Near x*, F₂(xᵏ) = O(‖eᵏ‖), so one Newton step reduces the inner residual to O(‖eᵏ‖²) — exactly the accuracy the outer step needs. A relaxation sweep (no Jacobian) only achieves O(‖eᵏ‖) inner residual and would degrade the outer convergence.

However, **asymptotic rate and path to the quadratic basin are distinct questions.** A tighter inner solve means w = M(xᵏ) is closer to x*, so F(w) is smaller and the outer Newton step starts from a better point. The practical effect is fewer outer SNES iterations before quadratic convergence dominates. The inner solve tolerance is therefore a tuning parameter that trades inner solve cost against outer iteration count — the right balance is problem-dependent and depends on how nonlinear the coupling is (which controls how far from x* the quadratic basin begins).

**Convergence summary for pure NLE** (NPC eliminates x₂ only; M = identity on x₁):

| Inner solve (x₂ only) | Outer Jacobian (acts on x₁) | Asymptotic rate | Path to quadratic basin |
|---|---|---|---|
| Few inner Newton iters | Exact Schur complement | Quadratic | Slower — more outer iters pre-asymptotically |
| x₂ fully converged | Exact Schur complement | Quadratic | Faster — fewer outer iters |
| Few inner Newton iters | Approx. Schur complement | Linear | Never enters quadratic basin |
| x₂ fully converged | Approx. Schur complement | Linear | Never enters quadratic basin |

**Convergence summary for nonlinear Gauss-Seidel** (inner SNES for all systems, multiplicative):

| Inner solve (both systems) | Outer Jacobian | Asymptotic rate | Path to quadratic basin |
|---|---|---|---|
| Few inner Newton iters | Full J(w) | Quadratic | Slower — more outer iters pre-asymptotically |
| Fully converged | Full J(w) | Quadratic | Faster — fewer outer iters |
| Few inner Newton iters | Block-diagonal J_diag(w) | Linear | Never enters quadratic basin |
| Fully converged | Block-diagonal J_diag(w) | Linear | Never enters quadratic basin |

In both cases, the outer Jacobian quality is the binding constraint on asymptotic convergence rate. Inner solve quality governs how efficiently the method reaches the regime where that asymptotic rate applies.

### Left vs. right nonlinear preconditioning

| | Left (PC_LEFT) | Right (PC_RIGHT) |
|---|---|---|
| Outer solver sees | F(M(x)) — preconditioned residual | F(x) — original residual |
| Jacobian used | J(M(x)) at preconditioned point | J(x) at current iterate |
| M applied | Before Newton linearization | After Newton direction computed |
| Effect | Better linearization point w | M as post-smoother/corrector |

Left is the classical nonlinear elimination formulation (Lanzkron et al.): apply inner solves first, then linearize at the improved point.

### Why this helps for thermofluid problems

With standard Newton, J has a large spectral contribution from the mechanical block that dominates δx, leaving T nearly unchanged each iteration. With left nonlinear elimination (multiplicative, two systems):

1. Inner solve 1: freeze T, solve F_mech(u, p | T) = 0 — mechanics converges
2. Inner solve 2: freeze (u, p), solve F_energy(T | u, p) = 0 — T converges
3. w = M(xᵏ) is a point where both subsystems are (approximately) satisfied
4. J(w) has a much more balanced spectrum — the outer Newton step updates all fields coherently

---

## What Is Already in Place

### Multiple nonlinear system infrastructure

MOOSE already has a complete multiple nonlinear system capability that provides nearly everything needed for the inner solves:

- **`nl_sys_names` / `solver_sys`**: Variables are explicitly partitioned across named nonlinear systems in the input file. The mapping is stored in `FEProblemBase::_solver_var_to_sys_num` (`framework/src/problems/FEProblemBase.C:324`).
- **One SNES per system**: Each `NonlinearSystem` owns an independent PETSc SNES object accessible via `NonlinearSystemBase::getSNES()`. These are exactly the inner SNES objects nonlinear elimination needs.
- **Frozen-field residual evaluation already works**: when system `nl0` evaluates its residual, variables assigned to `nl1` are held at their current solution values. The frozen-field mechanism is already functional at the system level.
- **`FEProblemSolve` fixed-point loop** (`framework/src/executioners/FEProblemSolve.C`): solves each system sequentially, iterating until a multi-system convergence criterion is met. This is multiplicative nonlinear Gauss-Seidel — exactly what `SNESCOMPOSITE` multiplicative mode implements.
- **`SteadySolve2`** (test executioner): demonstrates two-system sequential solve, confirming the infrastructure is exercised.
- **`PetscDMMoose`** (`framework/include/utils/PetscDMMoose.h`): provides `DMMooseSetSplitVars()` for per-system variable index sets — the IS and VecScatter objects needed by NASM/Composite.

**Key realization:** The existing multi-system fixed-point loop *is* nonlinear elimination — it just runs the inner solves to full convergence (Picard) rather than wiring them into PETSc as a nonlinear preconditioner with a global outer Newton step.

### Linear-level field splits

- **`FieldSplitPreconditioner`**: wraps PETSc `PC_FIELDSPLIT` for variable-subset KSP solves. Provides the input syntax pattern (Splits sub-blocks) we will follow.
- **`Split` hierarchy** (`framework/include/splits/Split.h`): additive, multiplicative, Schur splits at the KSP level.

### PETSc nonlinear preconditioning hooks

PETSc (in the checked-out submodule branch) has full infrastructure for nonlinear preconditioning:

| PETSc Function | Purpose |
|---|---|
| `SNESSetNPC(outer, inner)` | Attach `inner` SNES as nonlinear preconditioner of `outer` |
| `SNESSetNPCSide(snes, PC_LEFT)` | Left or right nonlinear preconditioning |
| `SNESSetNGS(snes, f, ctx)` | Register a nonlinear Gauss-Seidel routine |
| `SNESNASM` | Nonlinear Additive Schwarz (manages subdomain scatters explicitly) |
| `SNESCOMPOSITE` | Compose multiple SNES additively or multiplicatively |
| `SNESFAS` | Full Approximation Scheme (nonlinear multigrid) |
| `SNESNGMRES` | Nonlinear GMRES with left/right NPC support |

---

## What Is Missing

**A global outer SNES** that drives convergence of the combined system and treats the existing per-system SNES objects as nonlinear preconditioner components.

Currently, `FEProblemSolve` runs each per-system SNES to full convergence in a Picard loop — good for loose coupling but not exploiting PETSc's nonlinear preconditioning machinery. What's needed:

1. Compose the existing per-system SNES objects into a `SNESCOMPOSITE` (multiplicative) or register them via `SNESNASMSetSubdomains()`.
2. Attach that composite as the NPC of a global outer SNES via `SNESSetNPC()`.
3. The global outer SNES evaluates the combined residual (all variables) and takes a global Newton step after the inner preconditioner sweeps.
4. Inner solves use loose tolerances (one sweep or a few iterations), not full convergence — making each outer iteration cheaper than Picard.

No new frozen-field assembly machinery is needed. The existing per-system residual evaluation already holds other systems' variables fixed.

---

## Design

### Solve structure

Nonlinear elimination is not a named SNES type in PETSc — it falls out of the general `SNESCOMPOSITE` composition. The distinction between pure NLE and nonlinear Gauss-Seidel is simply which systems are included in the NPC composite:

- **All systems in NPC** → nonlinear Gauss-Seidel: every field gets an inner sweep before the outer Newton step
- **Only a subset of systems in NPC** → nonlinear elimination: non-included fields have no inner SNES; the outer Newton step acts on them with Schur-complement-quality Jacobian (since w = M(xᵏ) is identity on those DOFs)

The NPC composite is also fully general with respect to system type. A linear system Ax = b is F(x) = Ax − b = 0 with constant Jacobian — one KSP solve is an exact elimination for that system. PETSc's `SNESKSPONLY` type wraps a KSP in a SNES-compatible interface, so linear systems participate in `SNESCOMPOSITE` alongside nonlinear SNES objects without any special casing.

**Example — pure NLE** (mechanics excluded from NPC, energy and concentration eliminated):
```
Global outer SNES (all DOFs)
  └── NPC: SNESCOMPOSITE multiplicative
        ├── nl1 SNES         (variables: temperature)        ← eliminated
        │     residual: F_energy(T | u,p_frozen, c_frozen)
        └── lin0 SNESKSPONLY  (variables: concentration)     ← eliminated (linear, exact)
              residual: A(u,p,T)·c − b(u,p,T) = 0
# mechanics (u,p) has no inner SNES — outer Newton step handles it
# with Schur-complement-quality Jacobian
```

**Example — nonlinear Gauss-Seidel** (all systems in NPC):
```
Global outer SNES (all DOFs)
  └── NPC: SNESCOMPOSITE multiplicative
        ├── nl0 SNES         (variables: vel_x, vel_y, pressure)
        ├── nl1 SNES         (variables: temperature)
        └── lin0 SNESKSPONLY  (variables: concentration)
```

Each inner component updates its variable subset; updated values become the frozen state for subsequent components. The outer SNES then takes a global Newton step using the preconditioned iterate.

### Approach: leverage existing multi-system SNES objects

Rather than creating new inner SNES objects, we reuse the SNES objects already owned by each `NonlinearSystem`. The implementation becomes:

1. Retrieve `SNES inner_i = nl_sys[i]->getSNES()` for each system.
2. Create a `SNESCOMPOSITE` (or `SNESNASM`) and add each inner SNES.
3. Create a global outer SNES over all DOFs; attach the composite as its NPC.
4. The outer SNES needs a combined residual function and a combined Jacobian.

The global outer SNES is new. It operates on the concatenation of all per-system solution vectors. The combined residual is the concatenation of each system's residual evaluated at the current global state.

### Relationship to existing fixed-point loop

The existing `FEProblemSolve` fixed-point loop is a special case of the unified framework:
- Nonlinear systems with tight inner tolerances → full Newton convergence per system → Picard behavior
- Nonlinear systems with loose inner tolerances → nonlinear preconditioner sweep (NLE or NGS depending on `inner_nl_sys_names`)
- Linear systems wrapped in `SNESKSPONLY` → exact elimination in one KSP solve per NPC application

The `FEProblemSolve` fixed-point path is retained as a fallback for configurations not using `[NonlinearPreconditioning]`, but the unified outer SNES is the target architecture for all multi-system solves.

### Input syntax

The variable decomposition is already declared via `nl_sys_names` and `solver_sys`. A new `[NonlinearPreconditioning]` top-level block (peer to `[Preconditioning]`) configures the outer SNES and declares which systems participate in the NPC via `inner_nl_sys_names`. Systems listed there get an inner SNES in the composite; systems omitted are handled only by the outer Newton step.

Note: having a `[Preconditioning]` block for a system configures its linear PC and SNES options regardless — it does not imply inclusion in the NPC composite. The two concerns are independent.

**Pure NLE** (only `energy` eliminated; `mechanics` handled by outer Newton):
```ini
[Problem]
  nl_sys_names = 'mechanics energy'
[]

[NonlinearPreconditioning]
  inner_nl_sys_names = 'energy'      # mechanics has no inner SNES — pure NLE
  composition = multiplicative
  side = left
  petsc_options_iname = '-snes_type -snes_rtol'
  petsc_options_value  = 'newtonls 1e-8'
[]

[Preconditioning]
  [mechanics_pc]
    nl_sys = mechanics
    type = SMP
    petsc_options_iname = '-pc_type'
    petsc_options_value  = 'lu'
  []
  [energy_pc]
    nl_sys = energy
    type = SMP
    petsc_options_iname = '-pc_type -snes_rtol'
    petsc_options_value  = 'lu 1e-4'   # inner solve tolerance
  []
[]
```

**Nonlinear Gauss-Seidel** (all systems in NPC):
```ini
[NonlinearPreconditioning]
  inner_nl_sys_names = 'mechanics energy'   # all systems — NGS
  composition = multiplicative
  side = left
  petsc_options_iname = '-snes_type'
  petsc_options_value  = 'newtonls'
[]
```

Omitting `inner_nl_sys_names` entirely could default to NGS (all systems in NPC), making `[NonlinearPreconditioning]` a drop-in replacement for the existing fixed-point loop with a single parameter addition to switch to pure NLE.

---

## Implementation Plan

### Phase 1: Global combined residual and Jacobian

**Goal:** A global outer SNES can evaluate `F(x)` and `J(x)` over all DOFs by delegating to each system's existing assembly.

**Step 1a — Combined residual:**
1. Create a combined solution vector (PETSc `Vec`) spanning all per-system DOFs, or use the existing multi-system data structure if one exists.
2. Write a global residual callback that scatters the combined `x` into per-system vectors, calls each system's residual assembly (which already reads frozen fields from other systems), and gathers the per-system residual sub-vectors back into a combined `F`.
3. Verify: global residual matches the concatenation of per-system residuals for a two-system test problem.

**Step 1b — Monolithic global Jacobian:**

The scatter approach — assembling per-system diagonal blocks and then separately adding cross-system off-diagonal blocks via `ComputeJacobianBlocksThread` — requires non-trivial index remapping between per-system local DOF numbering and global DOF numbering, upfront sparsity allocation for cross-system entries, and correct routing of kernel contributions that are currently dropped. Getting any part wrong produces subtle bugs.

The simpler and more correct primary path is **monolithic assembly**: drive a single full Jacobian assembly pass over all variable pairs across all systems using the combined global matrix. Kernels already declare coupling to variables in other systems via `coupledValue` etc., so a full assembly pass naturally populates J₁₁, J₁₂, J₂₁, and J₂₂ in one shot — the same `ComputeFullJacobianThread` machinery used for single-system problems. The off-diagonal blocks emerge for free without any special handling.

Steps:
1. Allocate a combined global matrix with the full sparsity pattern across all systems' DOFs.
2. Write a global Jacobian callback that runs `ComputeFullJacobianThread` over all variable pairs from all systems, writing directly into the combined global matrix.
3. Verify: global Jacobian matches a finite-difference approximation of the global residual.
4. Verify: outer Newton converges for both weakly and strongly coupled two-system test problems.

The selective scatter approach (assemble per-system blocks and scatter) is a possible later optimization if monolithic assembly cost becomes a bottleneck, but should not be the first implementation.

### Phase 2: Compose inner SNES objects and wire outer NPC

**Goal:** Outer SNES uses existing per-system SNES objects as a multiplicative nonlinear preconditioner.

Steps:
1. After `FEProblemBase` has set up all nonlinear systems, retrieve their SNES objects.
2. Create a `SNESCOMPOSITE` and add each system's SNES via `SNESCompositeAddSNES()`, setting multiplicative mode.
3. Create the global outer SNES, attach the combined residual/Jacobian from Phase 1, and attach the composite as its NPC via `SNESSetNPC()` with `SNESSetNPCSide(outer, PC_LEFT)`.
4. Route the outer SNES into the solve path (replacing or wrapping the current per-system solve loop).
5. Verify: two-system thermofluid-like test converges; solution matches monolithic Newton reference.

### Phase 3: IS and scatter management for SNESNASM (optional extension)

**Goal:** Support overlap between variable sets for patch-based nonlinear elimination.

Steps:
1. Extract per-system variable IS from `PetscDMMoose` via `DMMooseSetSplitVars()`.
2. Build `VecScatter` objects for global ↔ per-system mappings.
3. Register as `SNESNASM` subdomains via `SNESNASMSetSubdomains()` as an alternative to `SNESCOMPOSITE`.

### Phase 4: Documentation and tests

1. Add a test in `test/tests/nonlinear_elimination/` with:
   - A two-physics problem demonstrating improved convergence over Picard (e.g., coupled diffusion-reaction where one field dominates the Newton direction).
   - A regression test confirming the same converged solution as monolithic Newton.
2. Add `framework/doc/content/source/problems/NonlinearElimination.md`.

---

## Key Files to Modify / Create

| File | Action |
|---|---|
| `framework/src/problems/FEProblemBase.C` | Parse `[NonlinearPreconditioning]` block; assemble global outer SNES after system setup |
| `framework/include/problems/FEProblemBase.h` | Add members for global outer SNES and NPC composite |
| `framework/src/executioners/FEProblemSolve.C` | Route through outer SNES when NLE is active instead of per-system fixed-point loop |
| `framework/include/utils/PetscDMMoose.h` / `.C` | Expose IS extraction helper if not already public |
| `test/tests/nonlinear_elimination/` | Create test directory |

No changes needed to `NonlinearSystemBase` residual/Jacobian assembly — the frozen-field behavior is already correct.

---

## Open Questions

1. **Left vs. right nonlinear preconditioning?** Left NPC (`PC_LEFT`) means the preconditioned residual drives convergence — the classical nonlinear elimination formulation. Right (`PC_RIGHT`) is a post-smoothing step. Verify against PETSc `SNESNGMRES` behavior and the Lanzkron et al. formulation.

2. **Global outer SNES identity**: A new SNES object is required. It operates over the combined DOF space of all nonlinear systems and owns the global residual/Jacobian from Phase 1. It cannot reuse an existing per-system SNES without conflicting with that system's solver configuration.

3. **Interaction with existing fixed-point loop**: The goal is a fully unified outer SNES composite over all solver systems — both SNES-based (nonlinear) and KSP-based (linear). A linear system Ax = b is just F(x) = Ax − b = 0 with a constant Jacobian; one KSP solve is an exact NLE elimination for that system, making it a natural and cheap participant in the NPC composite. PETSc's `SNESKSPONLY` type wraps a KSP solve in a SNES-compatible interface, allowing linear systems to be added to `SNESCOMPOSITE` alongside nonlinear SNES objects. With this, mixed NL+linear configurations are a special case of the general framework rather than a separate code path, and the multi-system fixed-point loop falls out naturally for all system combinations. The existing `FEProblemSolve` fixed-point path should be retained as a fallback for configurations not yet using `[NonlinearPreconditioning]`, but is not the target architecture.

5. **PETSc branch verification**: Confirm that the checked-out PETSc submodule branch includes `SNESSetNPC`, `SNESCOMPOSITE` multiplicative mode, and `SNESNASM`. (Per exploration, `nasm.c`, `composite/snescomposite.c`, and NPC hooks in `petscsnes.h` are present.)
