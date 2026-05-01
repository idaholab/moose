# interFoam Foundation v10 Implementation Extract

This note is a paraphrased extraction of the OpenFOAM Foundation v10 `interFoam`
implementation. It is meant as a working reference for parity work in this repo,
not as a verbatim copy of the upstream source.

## Scope

This extract focuses on the v10 two-phase incompressible `interFoam` solver
structure:

- startup and field creation
- mesh-motion and startup flux correction
- alpha / VOF transport ownership
- momentum prediction
- reduced-pressure correction
- data handoff points that matter for parity

## Source map

Solver-local files:

- `applications/solvers/multiphase/interFoam/interFoam.C`
- `applications/solvers/multiphase/interFoam/createFields.H`
- `applications/solvers/multiphase/interFoam/createFieldRefs.H`
- `applications/solvers/multiphase/interFoam/initCorrectPhi.H`
- `applications/solvers/multiphase/interFoam/correctPhi.H`
- `applications/solvers/multiphase/interFoam/alphaSuSp.H`
- `applications/solvers/multiphase/interFoam/rhofs.H`
- `applications/solvers/multiphase/interFoam/UEqn.H`
- `applications/solvers/multiphase/interFoam/pEqn.H`

Shared two-phase VOF files used by `interFoam`:

- `src/twoPhaseModels/twoPhaseMixture/VoF/alphaControls.H`
- `src/twoPhaseModels/twoPhaseMixture/VoF/createAlphaFluxes.H`
- `src/twoPhaseModels/twoPhaseMixture/VoF/alphaEqnSubCycle.H`
- `src/twoPhaseModels/twoPhaseMixture/VoF/alphaEqn.H`
- `src/twoPhaseModels/twoPhaseMixture/VoF/alphaScheme.H`

## Top-level execution order

The driver in `interFoam.C` is effectively:

```text
startup:
  createTime
  createMesh
  initContinuityErrs
  createDyMControls
  createFields
  createFieldRefs
  initCorrectPhi
  createUfIfPresent
  if not LTS:
    CourantNo
    setInitialDeltaT

time loop:
  readDyMControls
  if LTS:
    setRDeltaT
  else:
    CourantNo
    alphaCourantNo
    setDeltaT

  fvModels.preUpdateMesh()
  optionally store divU for mapped CorrectPhi
  mesh.update()
  if topology changed:
    clear cached previous alpha correction flux

  runTime++

  outer PIMPLE loop:
    optionally move mesh
    if mesh changed:
      rebuild gh and ghf
      MRF.update()
      correctPhi
      mixture.correct()
      optional meshCourantNo

    fvModels.correct()

    rhoPhi = 0
    alphaControls
    alphaEqnSubCycle
    turbulence.correctPhasePhi()
    mixture.correct()

    UEqn

    pressure loop:
      pEqn

    optional turbulence.correct()

  write
```

Three things are easy to miss:

- `rhoPhi` is created fresh before alpha transport inside each outer PIMPLE loop.
- alpha transport runs before `UEqn.H` and `pEqn.H` on every outer iteration.
- a topology change clears the cached previous-step alpha correction flux.

## Field creation and ownership

`createFields.H` establishes the reduced-pressure formulation:

- read `p_rgh`
- read `U`
- create `phi`
- construct `immiscibleIncompressibleTwoPhaseMixture mixture(U, phi)`
- construct the selected `twoPhaseChangeModel`
- build `rho` from the mixture and call `rho.oldTime()`
- read gravity, `hRef`, `gh`, and `ghf`
- reconstruct absolute pressure as `p = p_rgh + rho*gh`
- apply a pressure reference if `p_rgh.needReference()`
- mark `p_rgh` and `alpha1` as flux-required
- include `createAlphaFluxes.H`
- construct `incompressibleInterPhaseTransportModel turbulence(U, phi, alphaPhi1, mixture)`

`createFieldRefs.H` then binds the key references:

- `alpha2`
- `rho1`
- `rho2`
- `phaseChange`
- cached `rAU`

The practical takeaway is that the solved pressure variable is `p_rgh`.
Absolute pressure exists, but it is derived from `p_rgh` and hydrostatic head.

## Startup and mesh-motion flux correction

`initCorrectPhi.H` is not optional decoration. It is part of solver startup.

Behavior:

- on a fresh start, or when the selected phase-change model is `noPhaseChange`,
  `interFoam` performs an explicit startup `CorrectPhi` pass
- when `correctPhi` is enabled, it creates `rAU`, fixes `U/phi` boundary
  conditions, and calls `CorrectPhi(phi, U, p_rgh, interpolate(rAU), ...)`
- otherwise it still calls `CorrectPhi`, but with a unit scalar coefficient
- it includes `continuityErrs.H` immediately after

`correctPhi.H` is the mesh-motion analog:

- rebuild absolute face flux from `mesh.Sf() & Uf()`
- call `correctUphiBCs(U, phi, true)`
- call `CorrectPhi(...)`
- if a mapped `divU` exists, pass it in so divergence is preserved across mesh
  mapping
- include `continuityErrs.H`
- convert `phi` back to mesh-relative form

For parity work this matters because startup and mesh-change flux consistency are
handled by dedicated code paths, not by hoping the first pressure solve will
repair everything.

## Alpha / VOF transport

The alpha path is a mix of shared VOF infrastructure and solver-local helpers.

### Controls

`alphaControls.H` pulls these controls from the `alpha1` solver dictionary:

- `nAlphaCorr`
- `nAlphaSubCycles`
- `MULESCorr`
- `alphaApplyPrevCorr`

### Cached alpha flux state

`createAlphaFluxes.H` restores the stored alpha-flux field used across restart
and correction sweeps:

- it constructs `alphaPhi1Header`
- checks whether a stored alpha flux is present
- creates `alphaPhi1`
- allocates cached previous-correction storage `talphaPhi1Corr0`

That cached correction state is explicitly cleared in `interFoam.C` if the mesh
topology changes.

### Subcycling

`alphaEqnSubCycle.H` owns alpha subcycling:

- if `nAlphaSubCycles > 1`, it subcycles `alpha1`
- each subcycle includes `alphaEqn.H`
- it accumulates a time-weighted `rhoPhiSum`
- the final `rhoPhi` used downstream is the accumulated subcycled value
- if there is no subcycling, it just includes `alphaEqn.H` once

This is a direct statement of data ownership: alpha transport produces the
mass flux consumed later by the flow solve.

### Alpha equation structure

`alphaEqn.H` does the heavy lifting.

Main structure:

- include `alphaScheme.H` to build the discretization and compression scheme
- inspect `ddt(alpha)` and determine the off-centering coefficient
- allow Euler and local Euler directly
- allow Crank-Nicolson only when alpha subcycling is not active
- build a blended volumetric flux `phiCN` when off-centering is active
- include `alphaSuSp.H` for phase-change source terms and optional `divU`

`alphaSuSp.H` contributes:

- `Su` and `Sp` from `phaseChange.Salpha(alpha1)`
- an optional `divU` built from `phiCN`, including mesh flux when the mesh moves

Then `alphaEqn.H` splits in two main modes.

With `MULESCorr = true`:

- assemble and solve an upwind implicit transport equation for `alpha1`
- form an uncorrected alpha flux
- build a correction flux
- run the `nAlphaCorr` correction loop
- call bounded MULES correction routines
- under-relax correction updates after the first corrector

With `MULESCorr = false`:

- skip the implicit-upwind-plus-correction split
- run bounded explicit MULES transport directly

After the solve:

- `alpha2 = 1 - alpha1`
- previous-correction reuse can be applied through `alphaApplyPrevCorr`
- face densities from `rhofs.H` are used to rebuild `rhoPhi`
- `rhoPhi` uses the alpha-owned flux, not a later substitute reconstruction

The porting implication is straightforward: if parity is the goal, alpha owns
the authoritative density-weighted face flux.

## Momentum predictor (`UEqn.H`)

`UEqn.H` does the following:

- `MRF.correctBoundaryVelocity(U)`
- assemble
  `ddt(rho,U) + div(rhoPhi,U) + MRF.DDt(rho,U) + turbulence.divDevTau(rho,U)`
- place `phaseChange.SU(rho, rhoPhi, U)` and `fvModels.source(rho, U)` on the
  right-hand side
- relax the matrix
- apply `fvConstraints`
- if `pimple.momentumPredictor()` is enabled, solve with the reconstructed face
  force

The face force used in the momentum predictor is:

```text
mixture.surfaceTensionForce()
- ghf*snGrad(rho)
- snGrad(p_rgh)
```

That is an important reduced-pressure detail. Gravity enters through the
hydrostatic-density gradient term, not by solving for absolute pressure directly.

## Pressure correction (`pEqn.H`)

The pressure correction has the usual `HbyA` structure, but the exact pieces are
worth preserving:

- `rAU = 1 / UEqn.A()`
- `rAUf = interpolate(rAU)`
- `HbyA = constrainHbyA(rAU*UEqn.H(), U, p_rgh)`
- `phiHbyA = flux(HbyA) + moving-mesh ddtCorr term`
- make `phiHbyA` relative to MRF
- if needed, adjust `phiHbyA` to satisfy the pressure reference

Then `pEqn.H` adds the explicit face force:

```text
phig =
(
  mixture.surfaceTensionForce()
  - ghf*snGrad(rho)
) * rAUf * mesh.magSf()
```

and updates the predictor flux with:

```text
phiHbyA += phig
```

After that:

- `constrainPressure(p_rgh, U, phiHbyA, rAUf, MRF)`
- cache `Sp_rgh = phaseChange.Sp_rgh(rho, gh, p_rgh)`
- solve the non-orthogonal pressure loop
- on the final non-orthogonal iteration:
  - `phi = phiHbyA + p_rghEqn.flux()`
  - relax `p_rgh`
  - update
    `U = HbyA + rAU*reconstruct((phig + p_rghEqn.flux())/rAUf)`
  - correct `U` boundary conditions
  - apply `fvConstraints`

Finally it:

- includes `continuityErrs.H`
- corrects `Uf` for moving meshes
- makes `phi` relative to mesh motion
- reconstructs `p = p_rgh + rho*gh`
- reapplies the pressure reference if needed
- clears `rAU` when `correctPhi` is disabled

## What matters most for parity

If the goal is Foundation v10 `interFoam` parity, the highest-value structural
points are:

- alpha transport precedes momentum and pressure on each outer loop
- `rhoPhi` is alpha-owned and handed to `UEqn.H`
- `mixture.correct()` runs after mesh motion and again after alpha transport
- `p_rgh` is the primary solved pressure variable
- startup and mesh-motion flux consistency use explicit `CorrectPhi` passes
- the same gravity/surface-tension face-force idea appears in both `UEqn.H` and
  `pEqn.H`
- previous alpha-correction flux state is real solver state and survives across
  iterations unless a topology change invalidates it

## Minimal literal-port checklist

- keep a dedicated alpha-owned `rhoPhi`
- preserve `nAlphaCorr`
- preserve `nAlphaSubCycles`
- preserve `MULESCorr`
- preserve previous-correction reuse
- keep a true `p_rgh` / `p = p_rgh + rho*gh` split
- add an explicit startup `CorrectPhi` path
- keep gravity/surface-tension coupling on the same face-force form in both
  momentum prediction and pressure correction
- refresh mixture properties immediately after alpha transport

## Official v10 references

The primary upstream references used for this extract are the Foundation v10
source listings on `cpp.openfoam.org` and the Foundation `OpenFOAM-10` GitHub
repository, centered on the files listed in the source map above.
