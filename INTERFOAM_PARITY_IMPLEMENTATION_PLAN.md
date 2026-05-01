# InterFoam Parity Implementation Plan

## Goal

Achieve algorithmic parity, at the alpha-transport and mixture-coupling level, with the `interFoam`
free-surface solver workflow for the air-water dam-break target in `modules/navier_stokes`.

This does not mean matching OpenFOAM line-by-line. It means reproducing the same algorithmic
structure and the same key robustness mechanisms:

- bounded donor/base transport
- explicit high-order and compressive correction fluxes
- iterative correction limiting
- repeated `nAlphaCorr`
- true `nAlphaSubCycles`
- `rhoPhi` accumulation over alpha subcycles
- property rebuild after alpha transport and before pressure-velocity coupling
- stable startup for large density ratio

## Current Status

The branch now has:

- sharp-interface reduced-pressure flow physics in the live `modules/navier_stokes` tree
- a compileable sharp-interface VOF transport path
- donor/upwind alpha transport through the linear FV matrix path
- an explicit bounded correction stage with limiter iterations
- executioner-side support for volume-fraction systems and correction subcycles
- explicit face-flux state published from the sharp-interface VOF path:
  - `alpha_phi_bd`
  - `alpha_phi_ho`
  - `alpha_phi_corr`
  - `alpha_phi_limited`
  - `rho_phi`
- a first round of regression inputs under
  `modules/navier_stokes/test/tests/finite_volume/sharp_interface/vof_mules`
- a sharper discontinuous-step regression that exercises the correction loop on a non-smooth alpha
  field

The branch does **not** yet have full `interFoam` parity because:

- the custom MULES donor backbone is still under active debugging and is not yet trustworthy on a
  discontinuous sharp-step alpha field
- the alpha workflow is not yet the primary authoritative source of downstream mixture transport
- startup robustness pieces such as hydrostatic initialization and `CorrectPhi`-style cleanup are
  still missing

## Implementation Update: 2026-04-23

The current implementation state is more advanced than the original draft, but it also exposed one
specific blocker that must be resolved before claiming `interFoam`-style parity.

### What Is Implemented

- `SharpInterfaceVOFMULESCorrector` now owns published face-flux state for donor, high-order,
  correction, limited alpha flux, and accumulated `rho_phi`.
- `ReducedPressurePIMPLESolve` now supports dedicated volume-fraction systems and alpha subcycling.
- A first `interFoam`-style split exists:
  - donor/base alpha transport
  - explicit correction flux
  - limiter iterations
  - repeated correction sweeps
  - subcycle-level `rho_phi` accumulation
- the limiter loop now tightens a per-face `lambda` applied to the full raw correction flux,
  rather than incrementally accepting chunks of the remaining correction flux
- Regression inputs exist and compile/run in the repaired `moose_dev` environment.

### What The New Debugging Found

Instrumentation was added directly to `SharpInterfaceVOFMULESCorrector` to dump, on selected
interface faces:

- donor flux
- high-order flux
- correction flux
- accepted limiter factor
- element and neighbor alpha before/after correction
- donor-stage alpha before/after the explicit donor update

That instrumentation established the following:

1. The sharp-step failure is **not** primarily caused by the limiter stage.
   - On the inspected interface face, the correction stage behaved conservatively:
     `accepted_lambda = 0`, and the face remained a clean `0/1` jump.

2. The first bad state appears **before** the correction stage.
   - The donor-stage dump showed that the left boundary-adjacent control volume was already
     entering the donor update with `alpha = 1`, which is inconsistent with the intended
     sharp-step initial condition.

3. The custom donor backbone is therefore the current highest-priority algorithmic blocker.
   - The immediate problem is state ownership and initialization for the donor substep.
   - The custom explicit donor path is not yet guaranteed to start from the same authoritative
     alpha field that the rest of the transient system considers to be the old/subcycled state.

### Current Debugging Direction

The donor-stage state bug was closed by restoring the implicit donor backbone in the executioner
and keeping the user object responsible only for the bounded correction stage. The current
algorithmic focus has now shifted to the limiter itself:

- correction fluxes are rebuilt sweep-by-sweep from the current alpha field
- the limiter now computes and tightens a face `lambda` on the full raw correction flux
- bounded alpha updates are applied once per correction sweep from
  `alphaPhiBD + relaxation * lambda * alphaPhiCorr`

This is closer to `interFoam` than the earlier incremental “remaining-flux acceptance” approach,
but it is still not full parity with OpenFOAM's exact `MULES::limiter(...)` implementation.

### Implication For The Plan

This means the next parity milestone is no longer donor-state correctness. The limiter and
correction-flux work remains necessary, but the immediate blocker for a high-density-ratio
hydrostatic column and dam-break path is now reduced-pressure startup parity:

- true hydrostatic initialization of the solved pressure variable
- a `CorrectPhi`-style startup flux reconstruction from that seeded reduced-pressure field
- gravity-consistent pressure/flux boundary handling

### Current Task: Reduced-Pressure Hydrostatic Startup Parity

The current hydrostatic-column scaffold now keeps the interface present (`alpha_min = 0`,
`alpha_max = 1`), but the flow startup is still physically wrong: velocities blow up on the first
global step instead of remaining quiescent. This is the gating issue before the 2D dam-break
benchmark can be used as a meaningful parity target.

The current task is to make the reduced-pressure sharp-interface flow path follow `interFoam`
more literally at startup:

1. Treat the solved pressure variable as true `p_rgh`.
   - Seed a hydrostatically consistent reduced-pressure field before any startup cleanup or first
     pressure iteration.
   - Reconstruct total pressure only for diagnostics or reference handling if needed.

2. Split startup into a `createFields`-like stage and an `initCorrectPhi`-like stage.
   - First initialize reduced pressure, mixture density, and old/nonlinear state consistently.
   - Then rebuild gradients, predictor state, and face fluxes from the seeded field before normal
     SIMPLE/PIMPLE iterations.

3. Make the hydrostatic body-force balance use the same discrete operator in the predictor and the
   pressure/flux correction path.
   - Keep the reduced-pressure hydrostatic source explicit and consistent with the face-based
     pressure correction.
   - Ensure the face hydrostatic term uses the same `-gh * grad(rho)` logic that the momentum
     predictor assumes.

4. Add an explicit `CorrectPhi`-equivalent startup path.
   - Rebuild `Ainv`, `HbyA`, `phiHbyA`, constrained face mass flux, and cell velocity from the
     seeded reduced-pressure field in one pass.
   - Do not rely on repeated first-step pressure solves to discover hydrostatic balance.

5. Tighten pressure-boundary parity for gravity-driven closed-box startup.
   - Wall and zero-flux pressure treatment must remain consistent with gravity and reduced
     pressure, analogous to `fixedFluxPressure` behavior in `interFoam`.
   - Do not let the startup reconstruction impose a boundary pressure/flux relation that is
     inconsistent with the interior hydrostatic operator.

6. Keep alpha-owned `rhoPhi` authoritative.
   - The final subcycled `rhoPhi` published by alpha transport remains the only downstream
     density-weighted flux used by the flow solve.
   - Startup corrections must not silently replace it with an alternate density-flux
     reconstruction.

### Immediate Next Patch

The next code change should replace the current "seed pressure, then still solve pressure on
startup" behavior with a strict `CorrectPhi`-style reconstruction pass from seeded `p_rgh`, while
also making the wall pressure/flux treatment gravity-consistent.

### Verification Gate Before Dam-Break

Do not update the hydrostatic gold output or move on to the benchmark dam-break case until the
following sequence is satisfied:

1. The 2D hydrostatic column remains quiet for at least 1-3 steps.
   - `alpha_min = 0` and `alpha_max = 1` remain intact.
   - velocity extrema stay near machine zero instead of growing catastrophically.

2. The case includes a pressure check, not only velocity checks.
   - Verify reduced pressure and reconstructed total pressure are piecewise hydrostatic and
     compatible with the density jump.

3. Only after that should the hydrostatic-column CSV be blessed as a regression output.

4. Then use the hydrostatic case as the gate before a small 2D dam-break smoke test.

### Current State: 2026-04-24 Hydrostatic Pressure-Flux Audit

The reduced-pressure startup path has now been audited more directly at the face-flux level.

What was confirmed:

- suppressing the explicit sharp-interface hydrostatic pressure-equation source during seeded
  startup did **not** change the inferred pressure-diffusion flux on the representative horizontal
  interface face
- the large startup pressure flux is therefore not coming primarily from the extra explicit
  `capillary_hydrostatic_flux` divergence path
- on the representative internal face (`face_id = 209`), the stock pressure-diffusion term was
  decomposed as:
  - `p_elem ~= 0`
  - `p_neighbor ~= -4900.1`
  - `elem_matrix = 0.13985`
  - `elem_rhs = 0`
  - `reconstructed_p_grad_flux = 685.276`
- that means the dominant startup pressure flux on this face is matrix-driven by the seeded
  reduced-pressure jump, not by the diffusion RHS / nonorthogonal correction term

What was changed from that audit:

- the internal-face hydrostatic correction in `SharpInterfaceRhieChowMassFlux` now uses the same
  face matrix coefficient as the stock pressure-diffusion operator instead of an approximate
  projected `snGrad(rho)` form
- on the same representative face, the published explicit hydrostatic source flux now matches the
  stock pressure-diffusion magnitude:
  - `capillary_hydrostatic_source_flux = 685.276`

What this means:

- the internal interface-face hydrostatic cancellation is now using the correct operator and scale
- this was a real bug, and the audit closed it
- however, the 2D hydrostatic column is still not quiet enough to bless as a regression

Current physical status after the internal-face fix:

- the hydrostatic case still reaches `alpha_min = 0` and `alpha_max = 1` at `t = 0.01`
- startup velocities improved only slightly, for example:
  - before the internal-face operator fix:
    - `vel_x_max ~= 8.60e22`
    - `vel_y_min ~= -3.51e26`
  - after the internal-face operator fix:
    - `vel_x_max ~= 8.48e22`
    - `vel_y_min ~= -3.46e26`
- timestep 2 still fails after the first-step blow-up with PETSc `Absolute tolerance nan`

Narrowed next blocker:

- the remaining hydrostatic-startup failure is no longer primarily the interior interface-face
  hydrostatic operator
- the next audit target should be:
  1. wall / top-bottom reduced-pressure flux cancellation in the pressure BC path
  2. cell-velocity reconstruction consistency between the base Rhie-Chow update and the
     sharp-interface explicit hydrostatic correction

### Current State: 2026-04-24 Wall and Outer-Iteration Audit

The next audit closed one real cell-update bug and ruled out another likely boundary-path suspect,
but it also narrowed the remaining instability to the outer SIMPLE iterations.

What was confirmed:

- the top and bottom reduced-pressure wall-flux BC path is **not** the active startup bug
- representative wall faces were effectively quiet:
  - bottom wall representative face:
    - `reconstructed_p_grad_flux = 0`
    - `HbyA_source = 0`
    - `capillary_hydrostatic_source = 0`
    - `final_face_flux = 0`
  - top wall representative face:
    - `reconstructed_p_grad_flux ~= 0`
    - `HbyA_source ~= 0`
    - `capillary_hydrostatic_source ~= 0`
    - `final_face_flux = 0`
- the explicit sharp-interface cell-velocity correction was previously being written into
  `current_local_solution`, so it was lost on `update()`
- that was a real bug; the explicit correction now writes into the actual nonlinear system
  `solution` vector before `update()`

Representative evidence for the cell-correction bug:

- before the fix, a gas-side near-interface cell showed:
  - `base_velocity_y = 342.294`
  - `explicit_delta_y = -359.388`
  - `final_velocity_y = 342.294`
- after the fix, the same cell behavior became:
  - `base_velocity_y = 341.561`
  - `final_velocity_y = -17.8268`
- similarly, a liquid-side near-interface cell changed from effectively ignoring the explicit
  correction to:
  - `base_velocity_y = 0.342719`
  - `final_velocity_y = 0.0168674`

What the outer-iteration audit showed:

- the first startup pressure/velocity sweep is no longer the dominant failure mechanism
- the blow-up is created by repeated outer SIMPLE iterations
- representative timestep-1 behavior was:
  - with `num_iterations = 1`:
    - `vel_x_max ~= 6.60e-2`
    - `vel_y_max ~= 1.67e-2`
    - `vel_y_min ~= -1.76e1`
  - with `num_iterations = 2`:
    - `vel_x_max ~= 3.68e-1`
    - `vel_y_max ~= 3.13e2`
    - `vel_y_min ~= -7.71e-2`
  - with `num_iterations = 5`:
    - `vel_x_max ~= 2.13e2`
    - `vel_y_max ~= 2.78e1`
    - `vel_y_min ~= -1.94e6`
- removing the startup-only "reconstruct but do not solve pressure on SIMPLE iteration 1"
  shortcut did not materially improve the instability

Per-correction audit summary:

- the first two pressure-correction audits remained comparatively well behaved
- the runaway begins at audit step 3, which corresponds to the second outer SIMPLE iteration
- on the representative internal interface face (`face_id = 209`):
  - audit step 2:
    - `HbyA_source = 0.24941`
    - `reconstructed_p_grad_flux = 684.311`
    - `capillary_hydrostatic_source = 685.276`
    - `final_face_flux = -0.00445091`
  - audit step 3:
    - `HbyA_source = 1314.47`
    - `reconstructed_p_grad_flux = 1078.81`
    - `capillary_hydrostatic_source = 685.276`
    - `final_face_flux ~= -0.00445`
- on the representative gas-side near-interface cell:
  - audit step 2:
    - `final_velocity_y ~= -17.6`
  - audit step 3:
    - `HbyA_y = 5.25809`
    - `grad_p_y = -77191.2`
    - `base_velocity_y = 533.927`
    - `final_velocity_y = 174.539`
- later audit steps continue to grow rapidly with alternating sign, which is consistent with a
  runaway predictor/corrector state rather than a one-time startup imbalance

Narrowed next blocker after this audit:

- the remaining hydrostatic-startup failure is not primarily the wall pressure-flux BC path
- it is also no longer primarily the explicit cell-correction application bug
- the next audit target should be the momentum predictor / `HbyA` construction across outer SIMPLE
  iterations, especially in the gas-side near-interface cell where the runaway begins
  - the key question is which part of `HbyA` starts to diverge on the second outer iteration:
    off-diagonal momentum coupling, RHS forcing, transient treatment, or pressure-coupled state
    reuse

## Target Algorithm

The target alpha-transport workflow per global time step should be:

1. Split the global time step into `nAlphaSubCycles` substeps.
2. For each alpha substep:
   - build donor/bounded face flux `alphaPhiBD`
   - build high-order advective face flux
   - build compressive face flux
   - form correction flux `alphaPhiCorr = alphaPhiHO + alphaPhiComp - alphaPhiBD`
   - perform `nAlphaCorr` correction sweeps
   - in each sweep, run limiter iterations on `alphaPhiCorr`
   - update cell alpha from `alphaPhiBD + lambda * alphaPhiCorr`
   - clamp only as a last-resort safety net, not as the primary boundedness mechanism
3. Accumulate `rhoPhi` over the subcycles.
4. Rebuild mixture density/viscosity and any dependent functors from the final alpha.
5. Enter momentum-pressure coupling using the updated mixture state.

This is the minimum algorithmic bar for parity.

## Design Principles

### 1. Keep alpha transport as a dedicated subsystem

Do not try to force full `interFoam` parity entirely through the generic linear FV scalar
advection kernel. The alpha path now has enough special behavior that it should be treated as a
dedicated transport subsystem.

### 2. Make face-flux state explicit

Parity is difficult without explicit ownership of the alpha face fluxes. The implementation should
introduce dedicated face-centered state for:

- donor flux
- high-order flux
- correction flux
- limited final alpha flux
- accumulated `rhoPhi`

### 3. Let the executioner orchestrate, not compute everything

The executioner should control order:

- subcycles
- correction sweeps
- property rebuild timing

But the actual flux construction and limiting logic should live in a dedicated alpha transport
object.

## Required Implementation Work

## Phase 1: Refactor To Explicit Alpha Flux State

### Objective

Replace the current partially implicit alpha workflow with an explicit face-flux-driven transport
path.

### Tasks

- Add explicit face-centered storage for:
  - `alpha_phi_bd`
  - `alpha_phi_ho`
  - `alpha_phi_corr`
  - `alpha_phi_limited`
  - `rho_phi`
- Refactor the current `SharpInterfaceVOFMULESCorrector` into a more complete alpha transport
  driver object.
- Separate the following computations cleanly:
  - donor flux construction
  - high-order face reconstruction
  - compressive face flux
  - correction flux limiting
  - cell update

### Files Likely Affected

- `modules/navier_stokes/include/userobjects/SharpInterfaceVOFMULESCorrector.h`
- `modules/navier_stokes/src/userobjects/SharpInterfaceVOFMULESCorrector.C`
- `modules/navier_stokes/include/utils/`
- `modules/navier_stokes/include/physics/WCNSLinearFVSharpInterfaceVOFPhysics.h`
- `modules/navier_stokes/src/physics/WCNSLinearFVSharpInterfaceVOFPhysics.C`

### Acceptance Criteria

- The alpha transport driver owns explicit alpha flux state.
- Donor and correction fluxes are no longer implicit side effects of one generic solve.

## Phase 2: Implement True `nAlphaCorr`

### Objective

Match the repeated correction-loop structure used by `interFoam`.

### Tasks

- For each alpha correction sweep:
  - rebuild or refresh correction fluxes using the current alpha field
  - recompute limiter bounds
  - update alpha from the limited total flux
- Ensure the correction loop evolves alpha between sweeps instead of reusing one frozen correction
  field.
- Expose and honor:
  - `n_alpha_corrections`
  - `n_limiter_iterations`
  - correction relaxation controls

### Acceptance Criteria

- Multiple correction sweeps materially change the transported alpha result.
- Limiter iterations operate on the current correction flux state.

## Phase 2B: Match OpenFOAM-Style Limiter Budgets And Correction Flux Construction

### Objective

Improve the current face-`lambda` correction loop so it more literally follows OpenFOAM's MULES
budget accounting and correction-flux construction, which is necessary for stable high-density-
ratio free-surface scenarios.

### Tasks

- Replace the current ratio-based limiter tightening with explicit per-cell remaining allowance
  budgets for positive and negative correction.
- Update those budgets after each limiter pass from the currently limited correction flux field,
  rather than recomputing them as if the full raw correction were still available everywhere.
- Tighten the raw correction-flux construction so it is explicitly formed as:
  - bounded donor/base flux `alphaPhiBD`
  - higher-order advective flux contribution
  - compressive face flux contribution
  - raw correction `alphaPhiCorr = alphaPhiHO + alphaPhiComp - alphaPhiBD`
- Keep the compressive contribution algorithmically distinct from the advective high-order
  contribution until the raw correction is assembled, mirroring the OpenFOAM choreography more
  closely.
- Rebuild face alpha values and raw correction fluxes on each `nAlphaCorr` sweep from the current
  alpha state.
- Audit and tighten correction-side boundary handling for:
  - inflow alpha constraints
  - outflow correction treatment
  - wall-adjacent compression behavior
  - interface-normal usage near boundaries

### Files Likely Affected

- `modules/navier_stokes/include/userobjects/SharpInterfaceVOFMULESCorrector.h`
- `modules/navier_stokes/src/userobjects/SharpInterfaceVOFMULESCorrector.C`
- `modules/navier_stokes/test/tests/finite_volume/sharp_interface/vof_mules/*`

### Acceptance Criteria

- The limiter loop tracks explicit remaining positive/negative correction budgets per cell.
- The raw correction flux is assembled from clearly separated donor, high-order advection, and
  compression pieces.
- A discontinuous sharp-step regression remains bounded while showing nontrivial limited
  correction activity.

## Phase 3: Implement True `nAlphaSubCycles`

### Objective

Move from correction-only subcycling to full alpha subcycling.

### Tasks

- Split the global time step into alpha substeps.
- For each substep:
  - advance the full donor alpha equation from the correct authoritative old/subcycle state
  - rebuild high-order and compressive correction fluxes
  - perform correction limiting and update
- Ensure the subcycled alpha field is the authoritative state for the next substep.
- Remove any remaining mismatch between:
  - the vector used to build donor face fluxes
  - the vector updated by the donor backbone
  - the vector observed by the correction stage and postprocessors

### Files Likely Affected

- `modules/navier_stokes/include/executioners/ReducedPressurePIMPLESolve.h`
- `modules/navier_stokes/src/executioners/ReducedPressurePIMPLESolve.C`
- alpha transport driver user object

### Acceptance Criteria

- `volume_fraction_subcycles` causes full alpha re-advancement per substep.
- Alpha results change appropriately when subcycling is enabled.
- A sharp-step alpha field no longer enters the first donor subcycle from an inconsistent state.

## Phase 4: Implement `rhoPhi` Accumulation And Publish It

### Objective

Match `interFoam`’s use of alpha-subcycled fluxes to rebuild density transport consistently.

### Tasks

- Add `rhoPhi` accumulation over alpha substeps.
- Define the exact mixture-density interpolation used at faces.
- Publish `rhoPhi` to downstream consumers that need density-weighted flux transport.
- Ensure the final alpha transport stage controls the final `rhoPhi`.

### Acceptance Criteria

- `rhoPhi` is computed from the final limited alpha fluxes, not reconstructed loosely afterward.
- Mixture transport uses alpha-consistent flux data.

## Phase 5: Tighten Property Rebuild Order

### Objective

Ensure mixture properties are rebuilt at the correct time relative to alpha transport and
momentum-pressure coupling.

### Tasks

- After alpha subcycling completes:
  - rebuild `rho(alpha)`
  - rebuild `mu(alpha)` if applicable
  - refresh sharp-interface geometry dependencies that depend on alpha
- Ensure pressure-correction sees the final alpha-derived density state for that global step.

### Acceptance Criteria

- Flow physics always consumes the alpha-updated mixture state from the same time step.

## Phase 6: Startup Robustness For Dam-Break

### Objective

Match the practical stability protections that matter at large density ratio.

### Tasks

- Add hydrostatic initialization consistent with reduced pressure.
- Add a startup continuity cleanup stage analogous to `CorrectPhi`.
- Ensure the first time step sees:
  - stable alpha
  - consistent hydrostatic pressure
  - consistent face fluxes

### Acceptance Criteria

- Quiescent hydrostatic initialization does not generate large spurious startup velocities.

## Phase 7: Boundary Handling Parity

### Objective

Make alpha transport robust at inlet, outlet, and wall boundaries for free-surface runs.

### Tasks

- Audit Dirichlet alpha boundary handling in the correction stage.
- Ensure outflow behavior is consistent with bounded transport.
- Handle compressive fluxes near boundaries carefully.
- Validate behavior in closed-tank and outflow cases.

### Acceptance Criteria

- Boundary handling does not break boundedness or inject spurious interface distortion.

## Phase 8: Verification Ladder

### Objective

Validate the algorithm in the correct order before relying on the full dam-break benchmark.

### Test Sequence

1. 1D alpha translation
   - check boundedness
   - check mass conservation

2. Interface sharpening/compression sanity test
   - check sharpening without overshoot

3. Deforming interface advection case
   - check preservation quality and boundedness

4. Hydrostatic sharp-interface column
   - check startup stability and spurious currents

5. Small 2D dam-break smoke test
   - check stability and front evolution

6. Benchmark 2D dam-break case
   - compare front position and probe heights

### Acceptance Criteria

- Each lower-level test passes before the next one is used for debugging.

## Recommended File-Level Work Breakdown

### Alpha Transport Driver

- Expand:
  - `modules/navier_stokes/include/userobjects/SharpInterfaceVOFMULESCorrector.h`
  - `modules/navier_stokes/src/userobjects/SharpInterfaceVOFMULESCorrector.C`

### VOF Physics Wiring

- Update:
  - `modules/navier_stokes/include/physics/WCNSLinearFVSharpInterfaceVOFPhysics.h`
  - `modules/navier_stokes/src/physics/WCNSLinearFVSharpInterfaceVOFPhysics.C`

### Executioner Orchestration

- Update:
  - `modules/navier_stokes/include/executioners/ReducedPressurePIMPLESolve.h`
  - `modules/navier_stokes/src/executioners/ReducedPressurePIMPLESolve.C`

### Supporting Utilities

- Add or extend:
  - face-centered alpha flux storage utilities
  - mixture flux utilities
  - optional diagnostics/postprocessors for alpha flux budgets

## Short-Term Priority Order

1. Refactor alpha transport to explicit donor/correction face-flux state.
2. Implement true `nAlphaCorr`.
3. Implement true `nAlphaSubCycles`.
4. Accumulate and publish `rhoPhi`.
5. Tighten property rebuild order.
6. Add hydrostatic startup protections.
7. Add verification tests.
8. Run and tune the 2D dam-break benchmark.

## Definition Of Done

The implementation should be considered algorithmically at parity with `interFoam` when:

- alpha transport uses bounded donor plus limited correction fluxes
- repeated correction sweeps are implemented
- full alpha subcycling is implemented
- `rhoPhi` is accumulated consistently over subcycles
- mixture properties are rebuilt from final alpha before flow coupling
- hydrostatic startup is stable at large density ratio
- the 2D dam-break case runs robustly and produces physically reasonable results

## Current Focus: Outer Momentum-Predictor / Operator Rebuild (2026-04-27)

The remaining hydrostatic-column instability is now concentrated in the handoff from one outer
SIMPLE iteration to the next. The first and second outer corrections can stay close to the
expected hydrostatic branch, but the next outer momentum predictor still rebuilds an operator that
is not equivalent to `interFoam`'s `UEqn` / `pEqn` choreography.

### Why The Current Local Path Still Differs From `interFoam`

- the current MOOSE path assembles a full momentum system, including pressure-gradient and
  reduced-pressure sharp-interface source kernels, and solves it first
- `RhieChowMassFlux::computeHbyA()` then tries to reconstruct `HbyA` by subtracting pressure and
  explicit source terms back out of the already-assembled momentum matrix/RHS
- `interFoam`, by contrast, builds a relaxed non-pressure `UEqn`, uses that same operator for the
  optional momentum predictor solve, and then derives `rAU`, `HbyA`, `phiHbyA`, and `phig` from
  that same operator before the pressure correction loop

The current parity effort therefore needs a real predictor-operator split, not more tuning of the
post-hoc `HbyA` reconstruction.

### Patch 1: Introduce A Cached Predictor-Operator Path

Goal:

- add a first-class cache for the assembled/relaxed momentum predictor operator so the pressure
  corrector stops depending exclusively on live-system reverse-engineering

Scope:

1. Add cached predictor-operator storage in `RhieChowMassFlux`.
   - cache, per momentum component:
     - relaxed diagonal
     - base operator action `M*u - A*u - rhs`
   - keep the existing live-system reconstruction as a fallback path behind a flag

2. Populate that cache immediately after momentum predictor assembly.
   - after each real momentum predictor solve in `LinearAssemblySegregatedSolve`
   - after the startup assemble-only predictor path in `ReducedPressurePIMPLESolve`

3. Teach `computeHbyA()` to consume the cache when enabled.
   - use cached diagonal/base-operator data as the starting point for `Ainv`/`HbyA`
   - keep the existing pressure-gradient subtraction, explicit body-force re-addition, and
     SIMPLEC adjustment logic for now

4. Keep Patch 1 algebraically close to the existing implementation.
   - Patch 1 is infrastructure
   - it should not yet change which terms belong to the predictor operator versus explicit forcing

Acceptance target for Patch 1:

- the code path compiles, runs, and produces the same or nearly the same hydrostatic behavior as
  the current branch
- the pressure corrector can source `Ainv`/`HbyA` from a cached predictor-operator state instead
  of always rebuilding them directly from the current live system

### Patch 2: Split Pressure And Reduced-Pressure Forcing Out Of The Predictor Operator

Goal:

- make the cached predictor operator match OpenFOAM's pressure-free `UEqn` more literally

Scope:

1. Keep transient, advection, viscous, and friction terms in the predictor operator.
2. Move pressure-gradient and reduced-pressure sharp-interface forcing out of the predictor matrix
   / RHS assembly for the parity branch.
3. Rebuild `HbyA`, `phiHbyA`, and the predictor solve from that split operator plus explicit
   forcing, instead of solving the full coupled momentum equation and subtracting terms afterward.

Acceptance target for Patch 2:

- with `num_piso_iterations = 0`, the hydrostatic column remains on the correct branch through at
  least outer iteration 3

### Validation Order For The Operator Rebuild

1. Build and run the 1D boundedness control to make sure the predictor cache is behavior-neutral.
2. Run the hydrostatic column with:
   - `num_iterations = 2`, `num_piso_iterations = 0`, `num_steps = 1`
3. Run the same case with:
   - `num_iterations = 3`, `num_piso_iterations = 0`, `num_steps = 1`
4. Only then rerun the full first-step `num_iterations = 20` case.

### Direct `interFoam` Port Checklist

The next stage should be executed as a direct file-by-file port of the `UEqn.H` / `pEqn.H`
handoff rather than more local tuning. The goal is to make the state produced at the end of
`pEqn` the exact state expected by the next outer-loop momentum predictor.

1. `modules/navier_stokes/src/executioners/ReducedPressurePIMPLESolve.C`
   - make the outer loop follow `interFoam` literally:
     `alphaEqnSubCycle -> mixture/rho refresh -> UEqn -> while(correct) pEqn`
   - keep `correctPhi` logic startup-only
   - snapshot previous-iteration fields once at outer-loop entry, not inside the pressure
     corrector

2. `modules/navier_stokes/src/executioners/LinearAssemblySegregatedSolve.C`
   - make the cached predictor operator the real local `UEqn`
   - cache relaxed `A`, `rAU`, `H(U)`, and the explicit reconstructed force used by the momentum
     predictor
   - make the explicit force mirror
     `reconstruct((surfaceTensionForce - ghf*snGrad(rho) - snGrad(p_rgh))*magSf())`

3. `modules/navier_stokes/include/userobjects/RhieChowMassFlux.h`
4. `modules/navier_stokes/src/userobjects/RhieChowMassFlux.C`
   - promote exact face fields to first-class state:
     `rAUf`, `phiHbyA`, `phig`, and exact `pEqnFlux`
   - use them as the only source of truth for
     `phi = phiHbyA - pEqnFlux`
   - remove fallback paths that reconstruct the pressure correction indirectly from relaxed cell
     gradients

5. `modules/navier_stokes/include/userobjects/SharpInterfaceRhieChowMassFlux.h`
6. `modules/navier_stokes/src/userobjects/SharpInterfaceRhieChowMassFlux.C`
   - replace the current post-`pEqn` cell writeback with the local analog of
     `U = HbyA + rAU*reconstruct((phig - pEqnFlux)/rAUf)`
   - make that writeback consume the exact face `phig` and exact pressure-equation flux, not the
     current mixed `-HbyA_raw + delta` path

7. `modules/navier_stokes/src/linearfvbcs/LinearFVPressureFluxBC.C`
8. `modules/navier_stokes/src/linearfvbcs/LinearFVPressureSymmetryBC.C`
9. `modules/navier_stokes/src/physics/WCNSLinearFVSharpInterfaceFlowPhysics.C`
   - port `constrainPressure` semantics, not just its algebra
   - make pressure BC assembly consume a cached per-patch `snGrad(p)` state set immediately
     before the pressure solve, analogous to `fixedFluxPressure::updateCoeffs(snGradp)`

10. `modules/navier_stokes/src/userobjects/RhieChowMassFlux.C`
    - add a true post-writeback velocity boundary correction stage equivalent to
      `U.correctBoundaryConditions()`
    - make the next predictor read those corrected boundary values/fluxes

11. `modules/navier_stokes/src/executioners/ReducedPressurePIMPLESolve.C`
    - finish the alpha/outer-loop history model
    - keep timestep-old alpha fixed across the timestep
    - use a separate previous-outer state
    - preserve `alphaEqnSubCycle`-style `rhoPhi` accumulation semantics

12. `modules/navier_stokes/test/tests/finite_volume/sharp_interface/vof_mules/2d-hydrostatic-column.i`
13. `modules/navier_stokes/test/tests/finite_volume/sharp_interface/vof_mules/1d-sharp-vof-boundedness.i`
    - use these as the acceptance gates, in order:
      - `num_iterations = 3`, `num_piso_iterations = 0`, `num_steps = 1`
      - `num_iterations = 20`, `num_piso_iterations = 0`, `num_steps = 1`
      - 1D boundedness must remain unchanged

### Recommended Implementation Order

1. `RhieChowMassFlux.*`
2. `SharpInterfaceRhieChowMassFlux.*`
3. pressure BC files
4. `ReducedPressurePIMPLESolve.C`
5. `LinearAssemblySegregatedSolve.C`
6. tests

Highest-value target:

- get the post-`pEqn` `U` writeback to be the exact `interFoam` handoff state the next momentum
  predictor expects

### Current Thorough Port Steps

The remaining `1000:1` dam-break instability is now isolated enough that the next work should
proceed as a literal post-`pEqn` handoff port instead of more BC tuning.

1. `modules/navier_stokes/src/userobjects/SharpInterfaceRhieChowMassFlux.C`
   - stop using the corrected face-velocity field as the source of truth for cell `U`
   - restore cell writeback to the local analog of
     `U = HbyA + rAU*reconstruct((phig - pEqnFlux)/rAUf)`
   - keep the corrected face-velocity field only as a secondary outlet/boundary cache derived
     after cell writeback

2. `modules/navier_stokes/include/utils/FaceCenteredMapFunctor.h`
3. `modules/navier_stokes/src/utils/FaceCenteredMapFunctor.C`
   - reuse the existing Weller-style face-to-cell reconstruction as the local analog of
     `fvc::reconstruct`
   - keep the reconstruction source field face-scalar in spirit, represented locally as a
     face-vector field `psi_f * n_f`

4. `modules/navier_stokes/src/userobjects/RhieChowMassFlux.C`
   - keep `phi`, `phiHbyA`, `phig`, `pEqnFlux`, and `pressure_boundary_normal_gradient` as the only
     pressure-corrector source of truth
   - audit `|phi - phi(U)|` after each writeback stage and treat internal mismatch, not outlet
     flux, as the primary acceptance gate

5. `modules/navier_stokes/src/userobjects/SharpInterfaceRhieChowMassFlux.C`
   - once the reconstruct-based writeback is in place, rebuild the corrected outlet face cache from
     the final written-back cell field plus the solved face-normal flux, not the other way around

6. Acceptance gates
   - `2d-hydrostatic-column.i`: unchanged
   - `2d-dam-break-smoke.i`, low ratio: unchanged stable branch
   - `2d-dam-break-smoke.i`, `rho_l:rho_g = 1000:1`, one step:
     - `|phi - phi(U)|_2` must drop materially
     - the worst mismatch must stop being dominated by internal faces
     - top outlet target-flux sum must stay bounded through timestep 1
