# Diffuse-Interface Dam-Break Algorithm

This note documents the code path and numerical algorithm used by
`examples/dam_break/dam_break.i`. It focuses on the current reduced-pressure,
linear finite-volume, conservative diffuse-interface VOF path.

## Case Scope

The input `examples/dam_break/dam_break.i` configures a two-dimensional dam break:

- domain: `10a x 1.25a`
- mesh: `400 x 50`
- initial liquid column: `0 <= x < a`, `0 <= y < a`
- `a = 0.05715 m`
- liquid density: `rho_l = 998.19`
- gas density: `rho_g = 1.185`
- liquid dynamic viscosity: `mu_l = 1.0e-3`
- gas dynamic viscosity: `mu_g = 1.48e-5`
- gravity: `g = 9.81`
- timestep: `dt = 4.0e-4`
- VOF subcycles: `volume_fraction_subcycles = 1` (raised automatically if the alpha Courant limit demands)
- outer iterations: up to `num_iterations = 30`, stopping when the momentum and pressure residuals and the largest volume-fraction change are below `1e-6` (typically 3-4)
- PISO correctors: `num_piso_iterations = 2`, so three pressure corrections per outer iteration
- relaxation: `momentum_equation_relaxation = 0.9`, `pressure_variable_relaxation = 0.7`
- interface compression: `c_alpha = 0.01`
- alpha corrections: `n_alpha_corrections = 1`
- limiter iterations: `n_limiter_iterations = 6`
- startup projection: `startup_flux_corrections = 2`

The solve uses four linear systems:

```text
u_system
v_system
pressure_system
alpha_system
```

The executioner is `ReducedPressurePIMPLE`. The key physics objects are:

- `WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics`
- `WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics`
- `ConservativeDiffuseInterfaceRhieChowMassFlux`
- `ConservativeDiffuseInterfaceVOFMULESCorrector`

## Governing Variables

The transported volume fraction is

\begin{equation}
\alpha \in [0, 1],
\end{equation}

where `alpha = 1` denotes liquid and `alpha = 0` denotes gas. Mixture properties
are generated as functors (`WCNSLinearFVMixtureFunctorMaterial`):

\begin{equation}
\rho(\alpha) = \alpha \rho_l + (1 - \alpha)\rho_g,
\end{equation}

and

\begin{equation}
\mu(\alpha) = \alpha \mu_l + (1 - \alpha)\mu_g.
\end{equation}

The flow solve uses reduced pressure, commonly written as

\begin{equation}
p_{rgh} = p - \rho \mathbf{g}\cdot(\mathbf{x} - \mathbf{x}_{ref}).
\end{equation}

In the code, the pressure variable is named `pressure`, and the diffuse-interface
flow physics forces the dynamic/reduced-pressure path
(`solve_for_dynamic_pressure = true`). The reference point is the
`reference_pressure_point` parameter of the Rhie-Chow object, the origin by
default.

## High-Level Algorithm

For each timestep, the reduced-pressure PIMPLE solve keeps this ordering:

```text
begin timestep
  (first step only) startup state synchronization and continuity projection
  for each outer PIMPLE correction:
    advance outer-iteration histories
    freeze the lagged corrected face flux
    solve alpha with VOF subcycling
    publish alpha-consistent rho_phi
    solve momentum predictor
    solve pressure correction
    publish pressure-corrected velocity and face flux
  commit accepted transport history when the step is accepted
end timestep
```

The important correctness property is that alpha is solved inside the outer
PIMPLE loop, before the momentum-pressure correction. This ensures the current
outer iteration sees updated `alpha`, `rho_mixture`, `mu_mixture`,
`alpha_phi_limited`, and `rho_phi`.

## Object Construction Path

The input selects these physics blocks:

```text
[Physics/NavierStokes/ConservativeDiffuseInterfaceFlowSegregated/flow]
[Physics/NavierStokes/ConservativeDiffuseInterfaceVOFSegregated/vof]
```

The flow physics creates the conservative diffuse-interface Rhie-Chow object
itself, and refuses an existing stock `RhieChowMassFlux` on its blocks:

```text
WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics::addRhieChowUserObjects()
  -> addUserObject("ConservativeDiffuseInterfaceRhieChowMassFlux", rhieChowUOName(), params)
```

The VOF physics creates the MULES corrector:

```text
WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics::addUserObjects()
  -> addUserObject("ConservativeDiffuseInterfaceVOFMULESCorrector", prefix() + "vof_mules", params)
```

The VOF corrector receives:

```text
system_name = alpha_system
variable = alpha
face_flux = lagged_corrected_face_phi
compression_factor = c_alpha
interface_normal = flow_interface_unit_normal_face
n_alpha_corrections = 1
n_limiter_iterations = 6
liquid_density = rho_l
gas_density = rho_g
```

The VOF physics also creates the matrix advection kernel for alpha and, on each
outlet boundary of the flow physics, an inlet-outlet scalar boundary condition.
Both read the same frozen flux, so a face cannot be an outflow for the kernel
and an inflow for its boundary condition within one outer iteration:

```text
WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics::addScalarAdvectionKernels()
  -> addLinearFVKernel("LinearFVScalarAdvection", ..., face_flux = "lagged_corrected_face_phi")
WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics::addScalarOutletBC()
  -> addLinearFVBC("LinearFVInletOutletScalarBC", ..., face_flux = "lagged_corrected_face_phi")
```

## Published Face Fluxes

There are three central face-flux functors:

```text
corrected_face_phi
  pressure-corrected volumetric transport flux from Rhie-Chow

lagged_corrected_face_phi
  frozen volumetric flux used by the alpha equation during VOF transport

rho_phi
  alpha-consistent integrated mass flux published by the VOF corrector
```

The Rhie-Chow object owns `corrected_face_phi`, `lagged_corrected_face_phi` and
`previous_timestep_corrected_face_phi`, and the face fields of the pressure
equation, `phiHbyA`, `phig_flux` and `pressure_Ainv`. The VOF corrector owns
`alpha_phi_limited` and `rho_phi`.

The VOF corrector computes the integrated alpha flux

\begin{equation}
\Phi_{\alpha,f}
\end{equation}

and publishes the integrated density flux

\begin{equation}
\Phi_{\rho,f}
  = \rho_g \Phi_f + (\rho_l - \rho_g)\Phi_{\alpha,f},
\end{equation}

where:

- `phi_f` is the volumetric face flux density from `lagged_corrected_face_phi`
- `A_f` is the face measure
- `Phi_f = phi_f A_f` is the integrated volumetric flux
- `Phi_{\alpha,f}` is the limited integrated alpha flux

The flow physics advertises `rho_phi` as the momentum mass-flux functor
(`vof_rho_phi_functor`). The diffuse-interface Rhie-Chow object therefore uses
the VOF-owned `rho_phi` instead of reconstructing mass flux as a simple
face-density times volumetric-flux product.

## Timestep Entry And Startup Projection

`ReducedPressurePIMPLE` installs `ReducedPressurePIMPLESolve` as the inner solve.
At the beginning of the segregated solve loop:

```text
LinearAssemblySegregatedSolve::solve()
  -> ReducedPressurePIMPLESolve::initializeSolveLoop()
```

On the first time step (and only then, when `startup_continuity_projection` is
on) the reduced-pressure initialization path is:

```text
ReducedPressurePIMPLESolve::initializeSolveLoop()
  -> synchronizeSystemState(momentum systems)
  -> synchronizeSystemState(pressure system)
  -> synchronizeSystemState(alpha system)
  -> _problem.execute(EXEC_NONLINEAR)
  -> initializeStartupPressureField()
     -> RhieChowMassFlux::initPressureGradient()
     -> RhieChowMassFlux::prepareMomentumPredictor()
     -> assembleMomentumPredictorWithoutSolve()
     -> RhieChowMassFlux::initFaceMassFlux()
     -> correctStartupContinuityOnce() repeated startup_flux_corrections times
     -> _problem.execute(EXEC_NONLINEAR)
  -> ConservativeDiffuseInterfaceRhieChowMassFlux::seedTransportHistoryOnce()
```

`synchronizeSystemState()` copies a system's current solution into its old,
previous-iterate and all nonlinear states, so the first step's old solutions are
the initial condition and not zero. `seedTransportHistoryOnce()` commits the
transport history on the first solve of the run only; every later commit happens
on step acceptance.

The startup projection is intentionally pressure-field preserving:

```text
ReducedPressurePIMPLESolve::correctStartupContinuityOnce()
  -> PressureStateGuard saves the pressure vectors
  -> DiffuseInterfaceStartupProjectionGuard switches the hydrostatic, capillary
     and transient face fluxes off
  -> RhieChowMassFlux::computeHbyA()
  -> postPreparePressureCorrectorState()
  -> solvePressureCorrector()
  -> ConservativeDiffuseInterfaceRhieChowMassFlux::computeFaceMassFlux()
  -> restore the pressure vectors
  -> pressure_system.computeGradients()
```

The projection runs the first stages of the pressure corrector only: it repairs
the face flux before the first real outer correction without relaxing the
pressure or publishing a corrected velocity, and restores the user/equilibrium
initial reduced-pressure field.

## Outer PIMPLE Loop

The base segregated driver owns the outer loop:

```text
LinearAssemblySegregatedSolve::solve()
  -> initializeSolveLoop()
  -> while outer_iteration < num_iterations and not converged:
       preMomentumPressureIteration()
       RhieChowMassFlux::initPressureGradient() on the first iteration
       RhieChowMassFlux::prepareMomentumPredictor()
       solveMomentumPredictor()
       correctVelocity()
```

For this dam-break case:

```text
num_iterations = 30
num_piso_iterations = 2
momentum_absolute_tolerance = 1e-6
pressure_absolute_tolerance = 1e-6
active_scalar_absolute_tolerance = '1e-6'
```

So each timestep repeats the alpha update, the momentum predictor and the
pressure-correction stage until the normalized momentum and pressure residuals
and the largest change of the volume fraction since the previous outer
iteration are all below the tolerances, typically after 3-4 iterations; the
count is a cap. The volume fraction is judged by its change rather than by the
residual of its linear system, because the limited field never satisfies the
upwind system it was solved from, and the MULES limiter of a face may only
tighten from one outer iteration to the next, which keeps the loop from
cycling between two limiter states.

## Alpha Solve Ordering

The alpha solve is entered before the momentum predictor:

```text
ReducedPressurePIMPLESolve::preMomentumPressureIteration()
  -> advanceOuterIterationHistories()
  -> ConservativeDiffuseInterfaceRhieChowMassFlux::freezeLaggedCorrectedFaceFlux(first outer iteration)
  -> _problem.execute(EXEC_NONLINEAR)
  -> solveVolumeFractionSystems()
  -> _problem.execute(EXEC_NONLINEAR)
  -> store the volume-fraction changes as the active-scalar residuals
```

`freezeLaggedCorrectedFaceFlux()` copies either the previous timestep's accepted
corrected face flux or the current corrected face flux into
`lagged_corrected_face_phi`. The first outer iteration of every time step uses
the accepted flux of the previous step; later outer iterations use the current
corrected flux. Reading the accepted flux rather than the current one matters
when a step is solved again: the current flux then belongs to the rejected
attempt.

## Alpha Subcycling

The alpha solve can subdivide the global timestep:

\begin{equation}
N_s = \max\left(
  N_{configured},
  \left\lceil \frac{Co_\alpha}{Co_{\alpha,max}} \right\rceil
\right),
\end{equation}

with

\begin{equation}
\Delta t_s = \frac{\Delta t}{N_s}.
\end{equation}

The executioner temporarily mutates problem time for each subcycle:

\begin{equation}
t_{old,s} = t^n + s\Delta t_s,
\end{equation}

\begin{equation}
t_s = t^n + (s + 1)\Delta t_s.
\end{equation}

Subcycling (`N_s > 1`) requires a one-step time integrator such as
`ImplicitEuler`: a subcycle advances `solutionOld()` to the field of the
previous subcycle, but not the older solution a multi-step integrator such as
`BDF2` also reads, so the executioner refuses that combination.

The call path is:

```text
ReducedPressurePIMPLESolve::solveVolumeFractionSystems()
  -> ProblemTimeGuard saves dt, time and old time
  -> computeVolumeFractionSubcycles()
  -> solveOneVolumeFractionSystem()
     -> system.saveOldSolutions()
     -> setPreviousNewtonToCurrent()
     -> corrector.resetSubcycleFluxes()
     -> for each subcycle:
          runOneVolumeFractionSubcycle()
     -> system.restoreOldSolutions()
     -> setPreviousNewtonToCurrent()
     -> largest change of alpha from the previous outer iterate
  -> restore global problem time
  -> alpha_system.computeGradients()
  -> corrector.refreshPublishedRhoPhi()
```

Each subcycle performs:

```text
ReducedPressurePIMPLESolve::runOneVolumeFractionSubcycle()
  -> set problem dt, time and old time to the subcycle's
  -> advance solutionOld() to the current field, for subcycle > 0
  -> _problem.execute(EXEC_NONLINEAR)
  -> solveAdvectedSystem(alpha_system, ...)
  -> alpha_system.computeGradients()
  -> ConservativeDiffuseInterfaceVOFMULESCorrector::applyCorrection(dt_s / w_0, dt_s / dt)
```

The matrix solve is donor/upwind alpha transport. The explicit correction is
applied afterward by the MULES corrector. Its time scale is the subcycle step
divided by the leading coefficient `w_0` of the time integrator (`w_0 = 1` for
`ImplicitEuler`, `3/2` for `BDF2` at constant step), because the donor solve
weighted the new solution by `w_0 / dt_s`; the second argument is the fraction
of the global step the subcycle covers, used to accumulate the published fluxes.

The true timestep-old alpha is restored after subcycling. This matters because
transient terms and stateful objects must still see the real old-time state, not
the intermediate subcycle state.

## Donor Alpha Equation

The base matrix alpha solve represents the conservative transport equation

\begin{equation}
\frac{V_P}{\Delta t_s}(\alpha_P^* - \alpha_P^n)
+ \sum_f \Phi_{f}^{upwind}\alpha_{upwind} = 0,
\end{equation}

where:

- `V_P` is the cell volume
- `alpha_P^*` is the donor/upwind solution after the linear solve
- `Phi_f` is the integrated volumetric face flux from `lagged_corrected_face_phi`
- the matrix interpolation is upwind

The time derivative is written for `ImplicitEuler`; with `BDF2` the
`LinearFVTimeDerivative` kernel uses the integrator's weights instead.

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

\begin{equation}
\Phi_{\alpha,f}^{donor}
  = \Phi_f \alpha_{upwind}.
\end{equation}

For internal faces it computes a high-order face value using the Van Leer limited
face interpolation:

\begin{equation}
\alpha_f^{HO} = \alpha_f^{VanLeer}.
\end{equation}

Boundary faces get no compression. A closed face carries no correction and an
inflow face already carries its prescribed value. A fixed-value outflow face is
corrected to its boundary value, and with `outflow_two_term_expansion` an open
outflow face is corrected to the extrapolation
$\alpha_P + \nabla\alpha_P \cdot (\mathbf{x}_f - \mathbf{x}_P)$, clamped to
$[0, 1]$; otherwise it keeps the cell value of the donor flux. In the dam-break
input the top patch uses an inlet-outlet alpha condition; wall patches are
treated as closed for alpha flux.

For internal faces with nonzero transport flux, the compression term modifies
the high-order value:

\begin{equation}
\alpha_f^{comp}
  = bound\left[
      \alpha_f^{HO}
      + c_\alpha \operatorname{sign}(\Phi_f)
        \alpha_f^{lin}(1 - \alpha_f^{lin})
        (\mathbf{n}_{\alpha,f}\cdot\mathbf{n}_f)
    \right],
\end{equation}

where

\begin{equation}
\alpha_f^{lin}
  = bound\left[g_C\alpha_P + (1 - g_C)\alpha_N\right].
\end{equation}

The target alpha flux is then

\begin{equation}
\Phi_{\alpha,f}^{target}
  = \Phi_f \alpha_f^{comp}.
\end{equation}

The raw explicit correction is

\begin{equation}
\Phi_{\alpha,f}^{corr}
  = \Phi_{\alpha,f}^{target}
    - \Phi_{\alpha,f}^{working}.
\end{equation}

On the first correction sweep, the working flux is the donor flux. Later sweeps
use the previously accepted working flux.

The corresponding call stack is:

```text
ConservativeDiffuseInterfaceVOFMULESCorrector::applyCorrection()
  -> collectFaceCorrectionData()
     -> buildFaceCorrectionData()
        -> faceTransportData()
        -> boundaryValue() for boundary faces
        -> donorFlux()
        -> boundary faces: outflow target from the boundary value or the extrapolation
        -> internal faces: Van Leer limited face value (makeFace with LimiterType::VanLeer)
           and the compression calculation
```

## Limiter Math

For each cell, the corrector builds local admissible bounds from neighboring and
boundary alpha values, then limits the correction fluxes so the explicit update
does not leave those bounds.

The finite-volume explicit correction has the form

\begin{equation}
\alpha_P^{new}
  = \alpha_P^*
    - \frac{\Delta t_s / w_0}{V_P}
      \sum_{f \in \partial P} \lambda_f \Phi_{\alpha,f}^{corr},
\end{equation}

with face-orientation signs handled by the code through element/neighbor
updates and `w_0` the leading time-integrator coefficient defined above. The
limiter coefficient satisfies

\begin{equation}
0 \le \lambda_f \le 1.
\end{equation}

The available positive and negative correction budgets are built as

\begin{equation}
\psi_P^+
  = \frac{V_P w_0}{\Delta t_s}\max(0, \alpha_{P,max} - \alpha_P^*),
\end{equation}

and

\begin{equation}
\psi_P^-
  = \frac{V_P w_0}{\Delta t_s}\max(0, \alpha_P^* - \alpha_{P,min}).
\end{equation}

The code accumulates positive outflow and negative inflow correction sums for
each cell. It then iteratively tightens cell-side limiter estimates and applies
the minimum permitted value to each face. Only the process owning a cell sees
all of its faces, so the bounds, sums and limiters are computed for owned cells
only, and the limiters of cells next to a processor-partition face are sent to
the neighboring rank. Both adjacent ranks then accept the same face limiter,
and the result does not depend on the partitioning.

After limiting, the applied face correction is:

\begin{equation}
\Phi_{\alpha,f}^{limited}
  = w_k \lambda_f \Phi_{\alpha,f}^{corr},
\end{equation}

where

\begin{equation}
w_k =
\begin{cases}
1, & k = 0, \\
0.5, & k > 0.
\end{cases}
\end{equation}

For the dam-break input, `n_alpha_corrections = 1`, so only `w_0 = 1` is used.

The accepted face limiters are also kept per subcycle across the outer
iterations of a step, and a face limiter may only decrease from one outer
iteration to the next.

The limiter/update call stack is:

```text
ConservativeDiffuseInterfaceVOFMULESCorrector::applyCorrection()
  -> collectFaceCorrectionData()
  -> build local bounds of owned cells
  -> accumulate raw correction fluxes
  -> iterate n_limiter_iterations times:
       accumulate limited correction fluxes
       compute cell lambda_minus/lambda_plus of owned cells
       exchangePartitionCellLimiters()
       restrict accepted face lambda
  -> build limited_update vector
  -> current_local_solution.add(limited_update)
  -> publishFaceFluxes() on the final sweep
```

The published alpha flux is accumulated over subcycles with the same fraction
used for `rho_phi`:

\begin{equation}
\bar{\Phi}_{\alpha,f}
  = \sum_s \frac{\Delta t_s}{\Delta t}
      \Phi_{\alpha,f,s}^{limited}.
\end{equation}

## Alpha-Consistent Mass Flux

After correction, `publishFaceFluxes()` writes both:

```text
alpha_phi_limited
rho_phi
```

For each face and subcycle,

\begin{equation}
\Phi_{\rho,f,s}
  = \rho_g \Phi_{f,s}
    + (\rho_l - \rho_g)\Phi_{\alpha,f,s}^{limited}.
\end{equation}

The timestep-averaged published value is

\begin{equation}
\bar{\Phi}_{\rho,f}
  = \sum_s \frac{\Delta t_s}{\Delta t}\Phi_{\rho,f,s}.
\end{equation}

After all alpha systems finish, `solveVolumeFractionSystems()` restores the
global time, recomputes the alpha gradients and calls
`refreshPublishedRhoPhi()`, which recomputes `rho_phi` from the final published
`alpha_phi_limited` and the face state at the global time.

## Momentum Predictor

After alpha transport and nonlinear object execution, the base segregated solve
solves the momentum predictor:

```text
LinearAssemblySegregatedSolve::solve()
  -> solveMomentumPredictor()
     -> assembleMomentumPredictorOperator()
        -> RhieChowMassFlux::beginMomentumPredictorAssembly()
        -> computeLinearSystemSys()
        -> RhieChowMassFlux::momentumPredictorAssembled()
        -> NS::FV::relaxMatrix(), NS::FV::relaxRightHandSide()
        -> ConservativeDiffuseInterfaceRhieChowMassFlux::addMomentumPredictorExplicitForcing()
           -> reducedPressureMomentumPredictorForceDensity()
```

The conservative diffuse-interface flow physics configures the momentum equation
to use `rho_phi` as its mass flux and adds no pressure-gradient kernel. The
pressure, gravity and capillary forces enter the predictor only through the
explicit forcing, a cell vector reconstructed from face-normal scalars over all
faces of the cell:

\begin{equation}
\mathbf{f}_{p,rgh}
  \approx -\nabla p_{rgh}
           - gh_f \nabla_n \rho
           + \mathbf{f}_\sigma.
\end{equation}

The startup projection guard suppresses the hydrostatic, capillary and transient
face fluxes of the pressure equation during projection-only flux cleanup; the
normal solve enables them.

## Pressure Correction

`PIMPLESolve::correctVelocity()` runs the base pressure corrector once per PISO
stage (`num_piso_iterations = 2`, so three times per outer iteration). The base
corrector is an ordered sequence of stages:

```text
LinearAssemblySegregatedSolve::correctVelocity()
  -> ConservativeDiffuseInterfaceRhieChowMassFlux::computeHbyA()
  -> ReducedPressurePIMPLESolve::postPreparePressureCorrectorState()
     -> ConservativeDiffuseInterfaceRhieChowMassFlux::updateAdditionalPressureFluxFunctors()
     -> ConservativeDiffuseInterfaceRhieChowMassFlux::updateVelocityBoundaryState()
     -> ConservativeDiffuseInterfaceRhieChowMassFlux::updatePressureBoundaryNormalGradients()
  -> solvePressureCorrector()
     -> RhieChowMassFlux::cachePressureEquationFlux()
  -> ConservativeDiffuseInterfaceRhieChowMassFlux::computeFaceMassFlux()
  -> RhieChowMassFlux::preparePressureRelaxation()
  -> NS::FV::relaxSolutionUpdate(pressure)
  -> RhieChowMassFlux::finalizePressureCorrector()
     -> pressure_system.computeGradients()
     -> ConservativeDiffuseInterfaceRhieChowMassFlux::computeCellVelocity()
  -> ReducedPressurePIMPLESolve::postPublishPressureCorrectedState()
     -> ConservativeDiffuseInterfaceRhieChowMassFlux::updateVelocityBoundaryState()
```

`updateAdditionalPressureFluxFunctors()` builds the three face fields of the
pressure equation from the predictor of this outer iteration, all
area-integrated:

\begin{equation}
\Phi_f^{base} = -\left[(-H/A)_f\cdot\mathbf{n}_f + \phi_f^{ddt}\right] A_f,
\qquad
\Phi_f^{g} = \left[-gh_f \nabla_n\rho + \mathbf{f}_\sigma\cdot\mathbf{n}_f\right]
             (1/A)_{n,f} A_f,
\qquad
\Phi_f^{HbyA} = \Phi_f^{base} - \Phi_f^{g},
\end{equation}

with `pressure_Ainv` the face value of `1/A`, the coefficient of the pressure
equation's Laplacian. The face forces carry the same face-normal `1/A` as the
pressure gradient in the pressure flux, so that at equilibrium the pressure
flux cancels `phig_flux` face by face.

The transient term `phi^{ddt}` is the transient Rhie-Chow correction: for each old state the time integrator reads, the difference between
that step's accepted corrected flux and the face interpolation of its cell
velocity, weighted with the integrator's old-state weight
(`timeDerivativeRHSWeights()`, `1/dt` for `ImplicitEuler`, `2/dt` and
`-1/(2dt)` for `BDF2` at constant step), scaled by `(rho/A)_f` and faded out
where it is as large as the old flux itself. The Rhie-Chow object keeps the
corrected flux of the two last accepted steps for this purpose.

The corrected volumetric flux is then

\begin{equation}
\phi_f^{corr}
  = \frac{-\Phi_f^{HbyA} + \Phi_f^{p}}{A_f},
\end{equation}

where `Phi_f^p` is the flux the pressure equation added, cached after the
pressure solve. It is published as `corrected_face_phi`, and is expressed from
the stored predictor and pressure-equation fluxes rather than stored itself, so
a consumer cannot read a value that was correct for an earlier predictor state.

The mass flux stored in the inherited Rhie-Chow face-mass field is computed from
the corrected volumetric flux and the interpolated face density:

\begin{equation}
\dot{m}_f
  = \rho_f \phi_f^{corr}.
\end{equation}

For downstream conservative diffuse-interface advection, however, the flow physics
uses the VOF-published `rho_phi` functor. That distinction is deliberate:
`rho_phi` is tied to the limited alpha flux and is not assumed to equal
`rho_f phi_f`.

`computeCellVelocity()` writes the pressure-corrected velocity back to the
momentum solutions from the same all-face reconstruction the predictor forcing
used; the corrected face flux stays authoritative for continuity and transport
and is never projected back from the cell velocity.

## Transport History Commit

When the executioner accepts a step:

```text
ReducedPressurePIMPLE::takeStep()
  -> TransientBase::takeStep()
  -> if lastSolveConverged():
       ReducedPressurePIMPLESolve::commitAcceptedTimestepTransportHistory()
         -> ConservativeDiffuseInterfaceRhieChowMassFlux::commitAcceptedTimestepTransportHistory()
         -> ConservativeDiffuseInterfaceVOFMULESCorrector::commitAcceptedTimestepFluxes()
```

The Rhie-Chow object shifts the accepted `corrected_face_phi` into
`previous_timestep_corrected_face_phi` and the previous one into
`older_timestep_corrected_face_phi`, and stores the boundary pressure normal
gradients and face `1/A` of the converged step. The corrector stores the
published `alpha_phi_limited` and `rho_phi`. `timestepSetup()` of both objects
restores these at the start of every attempt at the next step, so the first
outer iteration of a step, including a repeated attempt at the same step, starts
from the last accepted state.

## Full Runtime Call Stack

The following stack omits framework scheduling details, but shows the relevant
dam-break path:

```text
ReducedPressurePIMPLE::ReducedPressurePIMPLE()
  -> _fixed_point_solve->setInnerSolve(ReducedPressurePIMPLESolve)

LinearAssemblySegregatedSolve::solve()
  -> ReducedPressurePIMPLESolve::initializeSolveLoop()
     -> (first step) synchronizeSystemState(every system), _problem.execute(EXEC_NONLINEAR)
     -> (first step) initializeStartupPressureField()
        -> RhieChowMassFlux::initPressureGradient()
        -> RhieChowMassFlux::prepareMomentumPredictor()
        -> assembleMomentumPredictorWithoutSolve()
        -> RhieChowMassFlux::initFaceMassFlux()
        -> correctStartupContinuityOnce() x startup_flux_corrections
           -> RhieChowMassFlux::computeHbyA()
           -> postPreparePressureCorrectorState()
           -> solvePressureCorrector()
           -> ConservativeDiffuseInterfaceRhieChowMassFlux::computeFaceMassFlux()
     -> ConservativeDiffuseInterfaceRhieChowMassFlux::seedTransportHistoryOnce()

  -> outer iteration 1
     -> ReducedPressurePIMPLESolve::preMomentumPressureIteration()
        -> advanceOuterIterationHistories()
        -> ConservativeDiffuseInterfaceRhieChowMassFlux::freezeLaggedCorrectedFaceFlux(true)
        -> _problem.execute(EXEC_NONLINEAR)
        -> solveVolumeFractionSystems()
           -> solveOneVolumeFractionSystem()
              -> ConservativeDiffuseInterfaceVOFMULESCorrector::resetSubcycleFluxes()
              -> runOneVolumeFractionSubcycle()
                 -> LinearAssemblySegregatedSolve::solveAdvectedSystem()
                 -> ConservativeDiffuseInterfaceVOFMULESCorrector::applyCorrection()
              -> runOneVolumeFractionSubcycle()
                 -> LinearAssemblySegregatedSolve::solveAdvectedSystem()
                 -> ConservativeDiffuseInterfaceVOFMULESCorrector::applyCorrection()
           -> alpha_system.computeGradients()
           -> ConservativeDiffuseInterfaceVOFMULESCorrector::refreshPublishedRhoPhi()
        -> _problem.execute(EXEC_NONLINEAR)
        -> store the volume-fraction changes as the active-scalar residuals

     -> RhieChowMassFlux::initPressureGradient()
     -> RhieChowMassFlux::prepareMomentumPredictor()
     -> solveMomentumPredictor()
        -> assembleMomentumPredictorOperator()
           -> ConservativeDiffuseInterfaceRhieChowMassFlux::addMomentumPredictorExplicitForcing()

     -> PIMPLESolve::correctVelocity()
        -> LinearAssemblySegregatedSolve::correctVelocity()
           -> ConservativeDiffuseInterfaceRhieChowMassFlux::computeHbyA()
           -> ReducedPressurePIMPLESolve::postPreparePressureCorrectorState()
              -> ConservativeDiffuseInterfaceRhieChowMassFlux::updateAdditionalPressureFluxFunctors()
              -> ConservativeDiffuseInterfaceRhieChowMassFlux::updateVelocityBoundaryState()
              -> ConservativeDiffuseInterfaceRhieChowMassFlux::updatePressureBoundaryNormalGradients()
           -> solvePressureCorrector()
              -> RhieChowMassFlux::cachePressureEquationFlux()
           -> ConservativeDiffuseInterfaceRhieChowMassFlux::computeFaceMassFlux()
           -> RhieChowMassFlux::preparePressureRelaxation()
           -> NS::FV::relaxSolutionUpdate(pressure)
           -> RhieChowMassFlux::finalizePressureCorrector()
              -> ConservativeDiffuseInterfaceRhieChowMassFlux::computeCellVelocity()
           -> ReducedPressurePIMPLESolve::postPublishPressureCorrectedState()

  -> outer iterations 2.. with freezeLaggedCorrectedFaceFlux(false)

ReducedPressurePIMPLE::takeStep()
  -> ReducedPressurePIMPLESolve::commitAcceptedTimestepTransportHistory()
```

With `volume_fraction_subcycles = 1` in the dam-break input, the
`runOneVolumeFractionSubcycle()` branch appears once per outer iteration;
the executioner raises the subcycle count when the alpha Courant number
would otherwise exceed its limit.

## Correctness Invariants

The implementation relies on these invariants:

1. Alpha transport runs before momentum-pressure coupling in every outer
   correction.
2. `lagged_corrected_face_phi` is frozen for alpha transport and is not changed
   in the middle of a subcycle.
3. The alpha matrix solve remains donor/upwind; bounded high-order behavior is
   supplied by the explicit limited correction.
4. `solutionOld()` for alpha is temporarily advanced between subcycles, then
   restored to the true timestep-old alpha.
5. `rho_phi` is published from the same limited alpha flux used to update alpha.
6. Startup continuity projection may repair face fluxes, but must restore the
   startup reduced-pressure field.
7. Pressure boundary normal gradients are refreshed after `HbyA` construction
   and before pressure equation assembly.
8. The accepted `corrected_face_phi` is committed on step acceptance so the next
   timestep can seed alpha transport and the transient Rhie-Chow correction
   consistently. The boundary pressure normal gradients and face `1/A` are
   committed with it, so the first momentum predictor of the next timestep
   applies the same boundary pressure force the converged step ended with.

## Files In The Path

- `modules/navier_stokes/examples/dam_break/dam_break.i`
- `modules/navier_stokes/src/executioners/ReducedPressurePIMPLE.C`
- `modules/navier_stokes/src/executioners/ReducedPressurePIMPLESolve.C`
- `modules/navier_stokes/src/executioners/PIMPLESolve.C`
- `modules/navier_stokes/src/executioners/LinearAssemblySegregatedSolve.C`
- `modules/navier_stokes/src/physics/WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics.C`
- `modules/navier_stokes/src/physics/WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics.C`
- `modules/navier_stokes/src/userobjects/ConservativeDiffuseInterfaceVOFMULESCorrector.C`
- `modules/navier_stokes/src/userobjects/ConservativeDiffuseInterfaceRhieChowMassFlux.C`
- `modules/navier_stokes/src/userobjects/RhieChowMassFlux.C`
