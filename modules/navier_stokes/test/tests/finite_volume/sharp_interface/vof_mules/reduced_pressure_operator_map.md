# Reduced-Pressure Sharp-Interface Operator Map

This note freezes the intended reduced-pressure operator chain against the current
sharp-interface MOOSE implementation before any more solver edits.

It is written to answer one question only:

`Where do we still differ discretely from the intended reduced-pressure sharp-interface path?`

## Intended Operator Contract

For the reduced-pressure split, the intended discrete chain is:

1. Momentum predictor
   Build the momentum operator and solve the predictor with a reconstructed
   face-normal source corresponding to
   `-snGrad(p_rgh) + surface_tension - gh*snGrad(rho)`.

2. Cell predictor state
   Form `HbyA` from the predictor solve using the same operator state that was
   used in the predictor RHS.

3. Face predictor flux
   Form `phiHbyA = flux(HbyA) + ddtCorr + phig`.

4. Pressure BC constraint
   Constrain the pressure boundary normal gradient from the same `phiHbyA` used
   in the pressure equation.

5. Pressure equation
   Solve the pressure correction from the divergence of the same predictor face
   flux, with the same face diffusion coefficient used by the BC constraint.

6. Face writeback
   Update the authoritative face flux from the pressure equation flux.

7. Cell writeback
   Update cell velocity from the face-consistent pressure correction, not from a
   different cell-side reinterpretation of the same sharp source.

The stability requirement is stronger than formula-level agreement:

- the same physical sharp source must remain in one consistent operator family
- face and cell representations must be equivalent up to the intended
  reconstruction
- BCs must see the same predictor face flux as the pressure equation

## Current MOOSE Chain

### 1. Momentum predictor RHS

Current code:

- `RhieChowMassFlux::addMomentumPredictorExplicitForcing`
- `ConservativeSharpInterfaceRhieChowMassFluxBase::populateMomentumPredictorPressureForceFaceField`
- `ConservativeSharpInterfaceRhieChowMassFluxBase::populateMomentumPredictorBodyForceFaceField`
- `ConservativeSharpInterfaceRhieChowMassFluxBase::reconstructFaceVectorFieldToCellSourceDensity`

Current behavior:

- The predictor RHS is assembled on cells.
- The pressure branch is reconstructed from a face scalar built from
  `_pressure_equation_flux / normal_ainv` for internal faces, or from the
  constrained boundary normal gradient on boundaries.
- The body branch is reconstructed from:
  - the lagged pressure-side `_capillary_hydrostatic_flux` once
    `_pressure_predictor_face_state_valid` is available
  - otherwise a local `gh * snGrad(rho)` style approximation
- Both branches are mapped back to cell vectors through
  `reconstructFixedFaceNormalScalarToCellVector`.

### 2. Predictor solve / HbyA assembly

Current code:

- `ReducedPressurePIMPLESolve::assembleMomentumPredictorOnly`
- `ReducedPressurePIMPLESolve::solveMomentumPredictor`
- `ReducedPressurePIMPLESolve::preparePressureCorrectorState`

Current behavior:

- The momentum systems are assembled and relaxed first.
- Sharp explicit forcing is added into the momentum RHS during predictor
  assembly.
- Only after the predictor stage does the sharp RC object build the
  pressure-side face predictor state in `updateAdditionalPressureFluxFunctors`.

### 3. Face predictor flux

Current code:

- `ConservativeSharpInterfaceRhieChowMassFlux::updateAdditionalPressureFluxFunctors`

Current behavior:

- `predictor_operator_phi` is built first.
- `transient_projection_flux` and `capillary_hydrostatic_flux` are built as face
  volumetric source fluxes.
- `phig_flux = transient_projection_flux + capillary_hydrostatic_flux`
- `pressure_predictor_base_phi = predictor_operator_phi + phig_flux`
- `phiHbyA_flux` is set to that same sharp predictor face flux.

This is the cleanest part of the current implementation.

### 4. Pressure BC constraint

Current code:

- `ReducedPressurePIMPLESolve::preparePressureCorrectorState`
- `WCNSLinearFVConservativeSharpInterfaceFlowPhysics::addInletBC`
- `WCNSLinearFVConservativeSharpInterfaceFlowPhysics::addOutletBC`
- `WCNSLinearFVConservativeSharpInterfaceFlowPhysics::addWallsBC`

Current behavior:

- Pressure BCs use `pressure_predictor_base_phi` as `HbyA_flux`
- Pressure BCs use `pressure_boundary_normal_gradient`
- `updatePressureBoundaryNormalGradients` is called after
  `updateAdditionalPressureFluxFunctors`

This is intentionally aligned with the intended contract.

### 5. Pressure equation

Current code:

- `WCNSLinearFVConservativeSharpInterfaceFlowPhysics::addPressureCorrectionKernels`
- `ReducedPressurePIMPLESolve::correctVelocityOnce`

Current behavior:

- The pressure equation uses anisotropic diffusion with `sharp_pressure_Ainv`
- The RHS divergence uses `pressure_predictor_base_phi`
- The old double-counting of separate transient / hydrostatic divergence terms
  has already been removed

This means the pressure equation and BC constraint now share the same predictor
face flux object.

### 6. Face writeback

Current code:

- `ConservativeSharpInterfaceRhieChowMassFlux::updateConservativePressureCoupledVelocityCorrectionFaceField`
- `RhieChowMassFlux::computeFaceMassFlux` through the RC interface

Current behavior:

- The live sharp pressure-correction branch is defined as
  `pressure_equation_flux - phig_flux`
- That branch is published as the authoritative pressure-coupled correction for
  face writeback

### 7. Cell writeback

Current code:

- `ConservativeSharpInterfaceRhieChowMassFlux::reconstructPressureCoupledCellVelocityDelta`
- `ConservativeSharpInterfaceRhieChowMassFlux::computeCellVelocity`

Current behavior:

- The cell velocity update is now
  `predictorVelocityComponent + pressureCoupledCellVelocityDelta`
- The older full-cell `-Ainv*grad(p)` writeback has been removed from the sharp
  path

This part is closer to the intended contract than before.

## Frozen Mismatch Table

| Stage | Intended discrete object | Current discrete object | Mismatch type | Evidence |
| --- | --- | --- | --- | --- |
| Predictor pressure source | Direct face `-snGrad(p_rgh)` operator in the same family as the pressure solve | `_pressure_equation_flux / normal_ainv` or constrained BC gradient | Wrong source object; pressure-equation artifact reused as predictor source | Predictor pressure branch is assembled from solved pressure-equation flux, not from a native predictor gradient operator |
| Predictor hydrostatic/body source | Same sharp face operator family as `phig` | Lagged `_capillary_hydrostatic_flux` after first pressure state; local `gh*snGrad(rho)` approximation before then | Mixed operator/state timing | Predictor stage and pressure-side source do not use the same operator/state on the first predictor pass |
| Predictor timing | Predictor and `phiHbyA` source built from the same state | Predictor RHS is assembled before pressure-side sharp predictor state exists | State-ordering mismatch | `assembleMomentumPredictorOnly` runs before `updateAdditionalPressureFluxFunctors` |
| Face-to-cell reconstruction | Reconstruction should preserve the original face-normal source in the intended least-squares sense | `reconstructFixedFaceNormalScalarToCellVector` solves a local normal matrix per cell with area weights | Unverified scaling / conservation mismatch | Benchmarks showed modest face fluxes but very large reconstructed cell body-force densities in interface-adjacent cells |
| Predictor/corrector pressure split | Predictor source and pressure correction should come from compatible operator families | Predictor pressure source uses `_pressure_equation_flux / normal_ainv`, while correction uses `pressure_equation_flux - phig_flux` | Inconsistent pressure ownership | Predictor and correction branches are not two views of the same primitive operator |
| First-step hydrostatic predictor source | Same source operator on step 1 and later steps | Falls back to local approximation until `_pressure_predictor_face_state_valid` | Startup inconsistency | Lagged shared-operator branch still follows the same late-time growth curve |

## Ranked Root-Cause Candidates

These are ranked by current evidence, not by implementation cost.

1. Predictor pressure branch uses the wrong primitive operator
   The current path derives predictor pressure forcing from
   `_pressure_equation_flux / normal_ainv`.
   That is a pressure-equation result, not a native predictor `snGrad(p_rgh)`
   operator.

2. Face-to-cell reconstruction amplifies the sharp source
   The unstable branches repeatedly showed modest face source magnitudes next to
   very large reconstructed cell predictor body-force densities along the
   interface-adjacent column.

3. Predictor hydrostatic branch uses inconsistent state timing
   The first predictor pass still does not use the same sharp hydrostatic source
   as the pressure-side `phig` path.

4. Predictor/source equivalence is not demonstrated on a frozen hydrostatic
   case
   The full dam-break is still being used too early as the first truth test.

## Minimal Audit Cases Before Any More Solver Edits

### Audit A: Frozen sharp hydrostatic predictor balance

Purpose:

- verify that the predictor source, `HbyA`, `phiHbyA`, and cell predictor state
  preserve near-hydrostatic balance with a fixed interface

Required diagnostics:

- predictor pressure source on faces
- predictor hydrostatic/body source on faces
- reconstructed cell predictor source
- `HbyA`
- `pressure_predictor_base_phi`
- corrected cell velocity after one pressure correction

Acceptance:

- no large interface-column cell-force amplification from an order-one face source
- no strong horizontal velocity generated from a hydrostatic initial state

### Audit B: One-step frozen pressure-correction audit

Purpose:

- verify that the predictor pressure source and the pressure correction are built
  from compatible operator families

Required diagnostics:

- face predictor pressure source
- face pressure equation flux
- face pressure-correction branch `pressure_equation_flux - phig_flux`
- reconstructed cell pressure-correction source
- corrected cell velocity

Acceptance:

- predictor and correction branches should cancel or balance on the frozen test
  without creating large residual horizontal motion

### Audit C: Short moving-interface toe audit

Purpose:

- validate the chosen operator fix on the real moving-interface geometry before
  rerunning the long benchmark

Required diagnostics:

- toe-row `u`
- toe `alpha=0.5` crossing
- face predictor source at the toe
- reconstructed cell predictor source on the toe-adjacent column
- `max_u`, `max_v`, `total_alpha`

Acceptance:

- positive early toe motion
- no large interface-column force amplification
- no onset of the previous `max_u` runaway by the early probe window

## Single Implementation Target To Try Next

Do not edit the solver until one target is selected from the mismatch table.

The highest-priority implementation target is:

`Replace the predictor pressure branch with a direct face reduced-pressure gradient operator in the same operator family as the pressure solve, then rerun Audit A and Audit B before touching the full benchmark.`

Reason:

- this is the clearest remaining discrete mismatch
- it does not depend on guessing about damping or time-step control
- it can be validated on frozen-interface audits before using the dam-break

The second target, only if the first one passes but the instability remains, is:

`Prove or fix conservation/scaling of reconstructFixedFaceNormalScalarToCellVector for sharp face-normal sources.`
