# Sharp-Interface Dam-Break Algorithm

This note documents the code path and numerical algorithm used by
`dam_break_openfoam_geometry.i`. It focuses on the current reduced-pressure,
linear finite-volume, conservative sharp-interface VOF path used as the
dam-break correctness gate.

## Case Scope

The input `dam_break_openfoam_geometry.i` configures a two-dimensional dam break:

- domain: `10a x 1.25a`
- mesh: `400 x 50`
- initial liquid column: `0 <= x < a`, `0 <= y < a`
- `a = 0.05715 m`
- liquid density: `rho_l = 998.19`
- gas density: `rho_g = 1.185`
- liquid dynamic viscosity: `mu_l = 1.0e-3`
- gas dynamic viscosity: `mu_g = 1.48e-5`
- gravity: `g = 9.81`
- timestep: `dt = 1.0e-4`
- VOF subcycles: `volume_fraction_subcycles = 2`
- alpha corrections: `n_alpha_corrections = 1`
- limiter iterations: `n_limiter_iterations = 6`

The solve uses four linear systems:

```text
u_system
v_system
pressure_system
alpha_system
```

The executioner is `ReducedPressurePIMPLE`. The key physics objects are:

- `WCNSLinearFVConservativeSharpInterfaceFlowPhysics`
- `WCNSLinearFVConservativeSharpInterfaceVOFPhysics`
- `ConservativeSharpInterfaceRhieChowMassFlux`
- `ConservativeSharpInterfaceVOFMULESCorrector`

## Governing Variables

The transported volume fraction is

$$
\alpha \in [0, 1],
$$

where `alpha = 1` denotes liquid and `alpha = 0` denotes gas. Mixture properties
are generated as functors:

$$
\rho(\alpha) = \alpha \rho_l + (1 - \alpha)\rho_g,
$$

and

$$
\mu(\alpha) = \alpha \mu_l + (1 - \alpha)\mu_g.
$$

The flow solve uses reduced pressure, commonly written as

$$
p_{rgh} = p - \rho \mathbf{g}\cdot(\mathbf{x} - \mathbf{x}_{ref}).
$$

In the code, the pressure variable is named `pressure`, but the sharp-interface
flow physics requires the dynamic/reduced-pressure path.

## High-Level Algorithm

For each timestep, the reduced-pressure PIMPLE solve keeps this ordering:

```text
begin timestep
  initialize reduced-pressure / VOF transport state
  for each outer PIMPLE correction:
    advance outer-iteration histories
    solve alpha with VOF subcycling
    publish alpha-consistent rhoPhi
    solve momentum predictor
    solve pressure correction
    publish pressure-corrected velocity and face flux
  commit accepted transport history
end timestep
```

The important correctness property is that alpha is solved inside the outer
PIMPLE loop, before the momentum-pressure correction. This ensures the current
outer iteration sees updated `alpha`, `rho_mixture`, `mu_mixture`,
`alpha_phi_limited`, and `rho_phi`.

## Object Construction Path

The input selects these physics blocks:

```text
[Physics/NavierStokes/ConservativeSharpInterfaceFlowSegregated/flow]
[Physics/NavierStokes/ConservativeSharpInterfaceVOFSegregated/vof]
```

The flow physics enforces the conservative sharp-interface Rhie-Chow object:

```text
WCNSLinearFVConservativeSharpInterfaceFlowPhysics
  -> rhieChowUserObjectType()
  -> "ConservativeSharpInterfaceRhieChowMassFlux"
```

The VOF physics creates the MULES corrector:

```text
WCNSLinearFVConservativeSharpInterfaceVOFPhysics::addUserObjects()
  -> addUserObject("ConservativeSharpInterfaceVOFMULESCorrector", "vof_mules", params)
```

The VOF corrector receives:

```text
system_name = alpha_system
variable = alpha
face_flux = vof_transport_phi
compression_factor = c_alpha
interface_normal = flow_interface_unit_normal_face
n_alpha_corrections = 1
n_limiter_iterations = 6
liquid_density = rho_l
gas_density = rho_g
```

The VOF physics also creates the matrix advection kernel for alpha:

```text
WCNSLinearFVConservativeSharpInterfaceVOFPhysics::addScalarAdvectionKernels()
  -> addLinearFVScalarAdvectionKernel(..., face_flux = "vof_transport_phi")
```

The top boundary gets an inlet-outlet scalar boundary condition, also driven by
`vof_transport_phi`.

## Published Face Fluxes

There are three central face-flux functors:

```text
corrected_face_phi
  pressure-corrected volumetric transport flux from Rhie-Chow

vof_transport_phi
  frozen volumetric flux used by the alpha equation during VOF transport

rho_phi
  alpha-consistent integrated mass flux published by the VOF corrector
```

The Rhie-Chow object owns `corrected_face_phi` and `vof_transport_phi`.
The VOF corrector owns `alpha_phi_limited` and `rho_phi`.

The VOF corrector computes the integrated alpha flux

$$
\Phi_{\alpha,f}
$$

and publishes the integrated density flux

$$
\Phi_{\rho,f}
  = \rho_g \Phi_f + (\rho_l - \rho_g)\Phi_{\alpha,f},
$$

where:

- `phi_f` is the volumetric face flux density from `vof_transport_phi`
- `A_f` is the face measure
- `Phi_f = phi_f A_f` is the integrated volumetric flux
- `Phi_{\alpha,f}` is the limited integrated alpha flux

The flow physics advertises `rho_phi` as the momentum mass-flux functor. The
sharp-interface Rhie-Chow object therefore uses the VOF-owned `rho_phi` instead
of reconstructing mass flux as a simple face-density times volumetric-flux
product.

## Timestep Entry And Startup Projection

`ReducedPressurePIMPLE` installs `ReducedPressurePIMPLESolve` as the inner solve.
At the beginning of the segregated solve loop:

```text
LinearAssemblySegregatedSolve::solve()
  -> ReducedPressurePIMPLESolve::initializeSolveLoop()
```

The reduced-pressure initialization path is:

```text
ReducedPressurePIMPLESolve::initializeSolveLoop()
  -> resetVOFTransportStateForNewSolve()
     -> ConservativeSharpInterfaceRhieChowMassFluxBase::clearVOFTransportState()
  -> initializeConsistentStartupState()
     -> synchronizeSystemState(momentum systems)
     -> synchronizeSystemState(pressure system)
     -> synchronizeSystemState(alpha system)
     -> _problem.execute(EXEC_NONLINEAR)
  -> initializeStartupPressureField()
     -> assembleMomentumPredictorWithoutSolve()
     -> RhieChowMassFlux::initFaceMassFlux()
     -> correctStartupContinuityOnce() repeated startup_flux_corrections times
  -> commitAcceptedVOFTransportHistoryIfNeeded()
```

The startup projection is intentionally pressure-field preserving:

```text
ReducedPressurePIMPLESolve::correctStartupContinuityOnce()
  -> PressureStateGuard saves pressure vectors
  -> SharpInterfaceStartupProjectionGuard suppresses startup-only source terms
  -> preparePressureCorrectorState(true)
  -> applyPressureCorrectionStage(recompute_face_mass_flux = true,
                                  publish_pressure_corrected_state = false)
  -> restore pressure vectors
  -> pressure_system.computeGradients()
```

The projection repairs face fluxes before the first real outer correction, but
restores the user/equilibrium initial reduced-pressure field.

## Outer PIMPLE Loop

The base segregated driver owns the outer loop:

```text
LinearAssemblySegregatedSolve::solve()
  -> initializeSolveLoop()
  -> while outer_iteration < num_iterations:
       preMomentumPressureIteration()
       solveMomentumPredictor()
       correctVelocity()
```

For this dam-break case:

```text
num_iterations = 1
num_piso_iterations = 0
num_pressure_nonorthogonal_correctors = 0
```

So each timestep performs one alpha update, one momentum predictor, and one
pressure-correction stage after startup initialization.

## Alpha Solve Ordering

The alpha solve is entered before the momentum predictor:

```text
ReducedPressurePIMPLESolve::preMomentumPressureIteration()
  -> advanceOuterIterationHistories()
  -> solveVolumeFractionBeforeFlowCorrection()
```

The VOF preparation and solve path is:

```text
ReducedPressurePIMPLESolve::solveVolumeFractionBeforeFlowCorrection()
  -> prepareVOFTransportStateForOuterIteration()
     -> clearVOFTransportState()
     -> freezeVOFTransportState(use_previous_timestep_transport_flux)
  -> _problem.execute(EXEC_NONLINEAR)
  -> solveVolumeFractionSystems()
  -> adoptPublishedVOFTransportState()
  -> _problem.execute(EXEC_NONLINEAR)
  -> storeActiveScalarResiduals()
```

`freezeVOFTransportState()` copies either the previous timestep corrected face
flux or the current corrected face flux into `vof_transport_phi`. On the first
timestep and first outer iteration, the previous timestep flux is used. Later
outer states use the current corrected flux.

## Alpha Subcycling

The alpha solve can subdivide the global timestep:

$$
N_s = \max\left(
  N_{configured},
  \left\lceil \frac{Co_\alpha}{Co_{\alpha,max}} \right\rceil
\right),
$$

with

$$
\Delta t_s = \frac{\Delta t}{N_s}.
$$

The executioner temporarily mutates problem time for each subcycle:

$$
t_{old,s} = t^n + s\Delta t_s,
$$

$$
t_s = t^n + (s + 1)\Delta t_s.
$$

The call path is:

```text
ReducedPressurePIMPLESolve::solveVolumeFractionSystems()
  -> computeVolumeFractionSubcycles()
  -> solveOneVolumeFractionSystem()
     -> system.saveOldSolutions()
     -> setPreviousNewtonToCurrent()
     -> corrector.resetSubcycleFluxes()
     -> for each subcycle:
          runOneVolumeFractionSubcycle()
     -> system.restoreOldSolutions()
     -> setPreviousNewtonToCurrent()
  -> restore global problem time
  -> finalizeVolumeFractionTransportState()
```

Each subcycle performs:

```text
ReducedPressurePIMPLESolve::runOneVolumeFractionSubcycle()
  -> setProblemSubcycleTime()
  -> advanceVolumeFractionSubcycleOldState() for subcycle > 0
  -> _problem.execute(EXEC_NONLINEAR)
  -> solveAdvectedSystem(alpha_system, ...)
  -> alpha_system.computeGradients()
  -> ConservativeSharpInterfaceVOFMULESCorrector::applyCorrection()
```

The matrix solve is donor/upwind alpha transport. The explicit correction is
applied afterward by the MULES corrector.

The true timestep-old alpha is restored after subcycling. This matters because
transient terms and stateful objects must still see the real old-time state, not
the intermediate subcycle state.

## Donor Alpha Equation

The base matrix alpha solve represents the conservative transport equation

$$
\frac{V_P}{\Delta t_s}(\alpha_P^* - \alpha_P^n)
+ \sum_f \Phi_{f}^{upwind}\alpha_{upwind} = 0,
$$

where:

- `V_P` is the cell volume
- `alpha_P^*` is the donor/upwind solution after the linear solve
- `Phi_f` is the integrated volumetric face flux from `vof_transport_phi`
- the matrix interpolation is upwind

The solve is performed by:

```text
LinearAssemblySegregatedSolve::solveAdvectedSystem()
  -> computeLinearSystemSys()
  -> relaxMatrix()
  -> relaxRightHandSide()
  -> linear_solver.solve()
  -> system.setSolution()
```

In the reduced-pressure VOF path, the inherited scalar lower limiter is disabled
for this solve. Boundedness is enforced through the limited face-flux correction.

## MULES Correction Target Flux

The corrector starts from the donor flux:

$$
\Phi_{\alpha,f}^{donor}
  = \Phi_f \alpha_{upwind}.
$$

For internal faces it computes a high-order face value using the Van Leer limited
face interpolation:

$$
\alpha_f^{HO} = \alpha_f^{VanLeer}.
$$

For boundary faces, the high-order value is selected from the active boundary
condition. In the dam-break input the top patch uses an inlet-outlet alpha
condition; wall patches are treated as closed for alpha flux.

For internal faces with nonzero transport flux, the compression term modifies
the high-order value:

$$
\alpha_f^{comp}
  = bound\left[
      \alpha_f^{HO}
      + c_\alpha \operatorname{sign}(\Phi_f)
        \alpha_f^{lin}(1 - \alpha_f^{lin})
        (\mathbf{n}_{\alpha,f}\cdot\mathbf{n}_f)
    \right],
$$

where

$$
\alpha_f^{lin}
  = bound\left[g_C\alpha_P + (1 - g_C)\alpha_N\right].
$$

The target alpha flux is then

$$
\Phi_{\alpha,f}^{target}
  = \Phi_f \alpha_f^{comp}.
$$

The raw explicit correction is

$$
\Phi_{\alpha,f}^{corr}
  = \Phi_{\alpha,f}^{target}
    - \Phi_{\alpha,f}^{working}.
$$

On the first correction sweep, the working flux is the donor flux. Later sweeps
use the previously accepted working flux.

The corresponding call stack is:

```text
ConservativeSharpInterfaceVOFMULESCorrector::applyCorrection()
  -> collectFaceCorrectionData()
     -> buildFaceCorrectionData()
        -> faceTransportData()
        -> donorFlux()
        -> highOrderFaceValue()
           -> sharedVanLeerFaceValue() for internal faces
        -> compression calculation
```

## Limiter Math

For each cell, the corrector builds local admissible bounds from neighboring and
boundary alpha values, then limits the correction fluxes so the explicit update
does not leave those bounds.

The finite-volume explicit correction has the form

$$
\alpha_P^{new}
  = \alpha_P^*
    - \frac{\Delta t_s}{V_P}
      \sum_{f \in \partial P} \lambda_f \Phi_{\alpha,f}^{corr},
$$

with face-orientation signs handled by the code through element/neighbor
updates. The limiter coefficient satisfies

$$
0 \le \lambda_f \le 1.
$$

The available positive and negative correction budgets are built as

$$
\psi_P^+
  = \frac{V_P}{\Delta t_s}\max(0, \alpha_{P,max} - \alpha_P^*),
$$

and

$$
\psi_P^-
  = \frac{V_P}{\Delta t_s}\max(0, \alpha_P^* - \alpha_{P,min}).
$$

The code accumulates positive outflow and negative inflow correction sums for
each cell. It then iteratively tightens cell-side limiter estimates and applies
the minimum permitted value to each face. On processor-partition faces, limiter
values are synchronized so both adjacent ranks use the same accepted face
limiter.

After limiting, the applied face correction is:

$$
\Phi_{\alpha,f}^{limited}
  = w_k \lambda_f \Phi_{\alpha,f}^{corr},
$$

where

$$
w_k =
\begin{cases}
1, & k = 0, \\
0.5, & k > 0.
\end{cases}
$$

For the dam-break input, `n_alpha_corrections = 1`, so only `w_0 = 1` is used.

The limiter/update call stack is:

```text
ConservativeSharpInterfaceVOFMULESCorrector::applyCorrection()
  -> build local bounds
  -> accumulate raw correction fluxes
  -> iterate n_limiter_iterations times:
       accumulate limited correction fluxes
       compute cell lambda_minus/lambda_plus
       restrict accepted face lambda
       synchronizePartitionFaceLimiters()
  -> build limited_update vector
  -> current_local_solution.add(limited_update)
  -> publishFaceFluxes() on final correction
```

The published alpha flux is accumulated over subcycles with the same fraction
used for `rho_phi`:

$$
\bar{\Phi}_{\alpha,f}
  = \sum_s \frac{\Delta t_s}{\Delta t}
      \Phi_{\alpha,f,s}^{limited}.
$$

## Alpha-Consistent Mass Flux

After correction, `publishFaceFluxes()` writes both:

```text
alpha_phi_limited
rho_phi
```

For each face and subcycle,

$$
\Phi_{\rho,f,s}
  = \rho_g \Phi_{f,s}
    + (\rho_l - \rho_g)\Phi_{\alpha,f,s}^{limited}.
$$

The timestep-averaged published value is

$$
\bar{\Phi}_{\rho,f}
  = \sum_s \frac{\Delta t_s}{\Delta t}\Phi_{\rho,f,s}.
$$

After all alpha systems finish, the executioner restores the global time and
calls:

```text
ReducedPressurePIMPLESolve::finalizeVolumeFractionTransportState()
  -> alpha_system.computeGradients()
  -> corrector.refreshPublishedRhoPhi()
```

`refreshPublishedRhoPhi()` recomputes `rho_phi` from the final published
`alpha_phi_limited` and current face state.

## Momentum Predictor

After alpha transport and nonlinear object execution, the base segregated solve
solves the momentum predictor:

```text
LinearAssemblySegregatedSolve::solve()
  -> solveMomentumPredictor()
```

The conservative sharp-interface flow physics configures the momentum equation
to use `rho_phi` as its mass flux. The momentum predictor also receives an
explicit reduced-pressure sharp-interface forcing through:

```text
ReducedPressurePIMPLESolve::addMomentumPredictorExplicitForcing()
  -> ConservativeSharpInterfaceRhieChowMassFlux::addMomentumPredictorExplicitForcing()
     -> reducedPressureMomentumPredictorForceDensity()
```

The force-density reconstruction includes pressure-gradient and, when not
suppressed, hydrostatic terms:

$$
\mathbf{f}_{p,rgh}
  \approx -\nabla p_{rgh}
           - gh_f \nabla_n \rho.
$$

The startup projection guard suppresses these startup-only source pieces during
projection-only flux cleanup, but the normal solve enables the hydrostatic
predictor path.

## Pressure Correction

The PIMPLE pressure-correction path is:

```text
PIMPLESolve::correctVelocity()
  -> storePressurePreviousOuterIterationState()
  -> preparePressureCorrectorState(subtract_updated_pressure = true)
     -> RhieChowMassFlux::computeHbyA()
     -> ReducedPressurePIMPLESolve::postPreparePressureCorrectorState()
        -> updateAdditionalPressureFluxFunctors()
        -> updateVelocityBoundaryState()
        -> updatePressureBoundaryNormalGradients(false)
  -> applyPressureCorrectionStage(recompute_face_mass_flux = true,
                                  publish_pressure_corrected_state = true)
     -> solvePressureCorrector()
     -> postPressureCorrectorSolve()
        -> pressure_system.computeGradients()
        -> cachePressureEquationFlux()
     -> publishPressureCorrectedState()
        -> computeFaceMassFlux()
        -> relaxPressureFieldForNextPredictor()
        -> computeCellVelocity()
        -> postPublishPressureCorrectedState()
```

The sharp-interface Rhie-Chow object constructs a predictor flux, pressure flux,
and pressure-corrected volumetric flux. Conceptually,

$$
\phi_f^{corr}
  = \phi_f^{H/A} + \phi_f^p,
$$

where `phiHbyA` is the predictor contribution and `phi_p` is the pressure
equation contribution. The corrected volumetric flux is written to
`corrected_face_phi`.

The mass flux stored in the inherited Rhie-Chow face-mass field is computed from
the corrected volumetric flux and the interpolated face density:

$$
\dot{m}_f
  = \rho_f \phi_f^{corr}.
$$

For downstream conservative sharp-interface advection, however, the flow physics
uses the VOF-published `rho_phi` functor. That distinction is deliberate:
`rho_phi` is tied to the limited alpha flux and is not assumed to equal
`rho_f phi_f`.

## Transport History Commit

At timestep end:

```text
ReducedPressurePIMPLE::postTakeStep()
  -> ReducedPressurePIMPLESolve::commitAcceptedTimestepTransportHistory()
     -> ConservativeSharpInterfaceRhieChowMassFluxBase::
        commitAcceptedTimestepTransportHistory()
```

This copies the accepted `corrected_face_phi` into
`previous_timestep_corrected_face_phi`. The next timestep can then seed the
first alpha transport state from the accepted previous-timestep flux.

## Full Runtime Call Stack

The following stack omits framework scheduling details, but shows the relevant
dam-break path:

```text
ReducedPressurePIMPLE::ReducedPressurePIMPLE()
  -> _fixed_point_solve->setInnerSolve(ReducedPressurePIMPLESolve)

LinearAssemblySegregatedSolve::solve()
  -> ReducedPressurePIMPLESolve::initializeSolveLoop()
     -> resetVOFTransportStateForNewSolve()
     -> initializeConsistentStartupState()
     -> initializeStartupPressureField()
        -> assembleMomentumPredictorWithoutSolve()
        -> RhieChowMassFlux::initFaceMassFlux()
        -> correctStartupContinuityOnce()
           -> PIMPLESolve::preparePressureCorrectorState()
           -> PIMPLESolve::applyPressureCorrectionStage()
              -> solvePressureCorrector()
              -> postPressureCorrectorSolve()
              -> ConservativeSharpInterfaceRhieChowMassFluxBase::computeFaceMassFlux()
     -> commitAcceptedVOFTransportHistoryIfNeeded()

  -> outer iteration 1
     -> ReducedPressurePIMPLESolve::preMomentumPressureIteration()
        -> advanceOuterIterationHistories()
        -> solveVolumeFractionBeforeFlowCorrection()
           -> prepareVOFTransportStateForOuterIteration()
              -> ConservativeSharpInterfaceRhieChowMassFluxBase::clearVOFTransportState()
              -> ConservativeSharpInterfaceRhieChowMassFluxBase::freezeVOFTransportState()
           -> _problem.execute(EXEC_NONLINEAR)
           -> solveVolumeFractionSystems()
              -> solveOneVolumeFractionSystem()
                 -> ConservativeSharpInterfaceVOFMULESCorrector::resetSubcycleFluxes()
                 -> runOneVolumeFractionSubcycle()
                    -> LinearAssemblySegregatedSolve::solveAdvectedSystem()
                    -> ConservativeSharpInterfaceVOFMULESCorrector::applyCorrection()
                 -> runOneVolumeFractionSubcycle()
                    -> LinearAssemblySegregatedSolve::solveAdvectedSystem()
                    -> ConservativeSharpInterfaceVOFMULESCorrector::applyCorrection()
              -> finalizeVolumeFractionTransportState()
                 -> ConservativeSharpInterfaceVOFMULESCorrector::refreshPublishedRhoPhi()
           -> ConservativeSharpInterfaceRhieChowMassFluxBase::adoptPublishedVOFTransportState()
           -> _problem.execute(EXEC_NONLINEAR)

     -> solveMomentumPredictor()
        -> ReducedPressurePIMPLESolve::addMomentumPredictorExplicitForcing()
           -> ConservativeSharpInterfaceRhieChowMassFlux::
              addMomentumPredictorExplicitForcing()

     -> PIMPLESolve::correctVelocity()
        -> PIMPLESolve::preparePressureCorrectorState()
           -> RhieChowMassFlux::computeHbyA()
           -> ConservativeSharpInterfaceRhieChowMassFluxBase::
              updateAdditionalPressureFluxFunctors()
           -> RhieChowMassFlux::updateVelocityBoundaryState()
           -> RhieChowMassFlux::updatePressureBoundaryNormalGradients()
        -> PIMPLESolve::applyPressureCorrectionStage()
           -> solvePressureCorrector()
           -> PIMPLESolve::postPressureCorrectorSolve()
              -> ConservativeSharpInterfaceRhieChowMassFluxBase::cachePressureEquationFlux()
           -> PIMPLESolve::publishPressureCorrectedState()
              -> ConservativeSharpInterfaceRhieChowMassFluxBase::computeFaceMassFlux()
              -> PIMPLESolve::relaxPressureFieldForNextPredictor()
              -> ConservativeSharpInterfaceRhieChowMassFluxBase::computeCellVelocity()
              -> ReducedPressurePIMPLESolve::postPublishPressureCorrectedState()

ReducedPressurePIMPLE::postTakeStep()
  -> ReducedPressurePIMPLESolve::commitAcceptedTimestepTransportHistory()
```

Because `volume_fraction_subcycles = 2` in the dam-break input, the
`runOneVolumeFractionSubcycle()` branch appears twice per timestep.

## Correctness Invariants

The implementation relies on these invariants:

1. Alpha transport runs before momentum-pressure coupling in every outer
   correction.
2. `vof_transport_phi` is frozen for alpha transport and is not changed in the
   middle of a subcycle.
3. The alpha matrix solve remains donor/upwind; bounded high-order behavior is
   supplied by the explicit limited correction.
4. `solutionOld()` for alpha is temporarily advanced between subcycles, then
   restored to the true timestep-old alpha.
5. `rho_phi` is published from the same limited alpha flux used to update alpha.
6. Startup continuity projection may repair face fluxes, but must restore the
   startup reduced-pressure field.
7. Pressure boundary normal gradients are refreshed after `HbyA` construction
   and before pressure equation assembly.
8. The accepted `corrected_face_phi` is committed at timestep end so the next
   timestep can seed alpha transport consistently.

## Files In The Path

- `dam_break_openfoam_geometry.i`
- `modules/navier_stokes/src/executioners/ReducedPressurePIMPLE.C`
- `modules/navier_stokes/src/executioners/ReducedPressurePIMPLESolve.C`
- `modules/navier_stokes/src/executioners/PIMPLESolve.C`
- `modules/navier_stokes/src/executioners/LinearAssemblySegregatedSolve.C`
- `modules/navier_stokes/src/physics/WCNSLinearFVConservativeSharpInterfaceFlowPhysics.C`
- `modules/navier_stokes/src/physics/WCNSLinearFVConservativeSharpInterfaceVOFPhysics.C`
- `modules/navier_stokes/src/userobjects/ConservativeSharpInterfaceVOFMULESCorrector.C`
- `modules/navier_stokes/src/userobjects/ConservativeSharpInterfaceRhieChowMassFlux.C`
- `modules/navier_stokes/src/userobjects/ConservativeSharpInterfaceRhieChowMassFluxBase.C`
