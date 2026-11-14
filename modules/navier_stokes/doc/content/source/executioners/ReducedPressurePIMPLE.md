# ReducedPressurePIMPLE

!syntax description /Executioner/ReducedPressurePIMPLE

## Overview

`ReducedPressurePIMPLE` is the transient executioner of the two-phase diffuse-interface flow set up
by [WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics](WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics.md)
and [WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics](WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics.md).
It follows the [PIMPLE](PIMPLE.md) algorithm, solving for the reduced pressure
$p_{rgh} = p - \rho\, \mathbf{g} \cdot \left( \mathbf{x} - \mathbf{x}_{ref} \right)$, with the
volume fraction solved inside every outer iteration. The full algorithm is described in the
[diffuse-interface dam break algorithm](diffuse_interface_dam_break_algorithm.md) note.

Each outer iteration of a time step:

1. Freezes the face flux `lagged_corrected_face_phi` that the volume-fraction transport and the
   outlet backflow switches read during this iteration. The first outer iteration of a step uses
   the flux of the last accepted step.
2. Solves each volume-fraction system listed in `active_scalar_systems`. A system is advanced in
   [!param](/Executioner/ReducedPressurePIMPLE/volume_fraction_subcycles) subcycles, raised to
   $\lceil C / C_{max} \rceil$ when the Courant number $C$ of the face flux exceeds
   [!param](/Executioner/ReducedPressurePIMPLE/volume_fraction_max_courant). Each subcycle is an
   upwind solve followed by the bounded correction of the system's
   [ConservativeDiffuseInterfaceVOFMULESCorrector](ConservativeDiffuseInterfaceVOFMULESCorrector.md);
   a system without exactly one such corrector is an error.
3. Solves the momentum predictor and the reduced-pressure correction, with
   [!param](/Executioner/ReducedPressurePIMPLE/num_piso_iterations) additional PISO stages.
4. Solves the energy system, when there is one, and then recovers the temperature from it.

The outer loop stops once the momentum and pressure residuals and the largest change of each volume
fraction since the previous outer iteration are all below their absolute tolerances, or after
[!param](/Executioner/ReducedPressurePIMPLE/num_iterations) iterations. The volume fraction is
judged by its change rather than by the residual of its linear system, because the limited field
never satisfies the upwind system it was solved from.

After each accepted time step, the face flux and the limited volume-fraction fluxes are committed
as the history the next step starts from; a step that is repeated starts again from that history.

## Startup continuity projection

The initial velocity of a two-phase case usually does not satisfy continuity together with the
initial reduced pressure. With
[!param](/Executioner/ReducedPressurePIMPLE/startup_continuity_projection) `= true`, the first time
step begins with [!param](/Executioner/ReducedPressurePIMPLE/startup_flux_corrections)
pressure-only projections that repair the face flux and then restore the user-supplied reduced
pressure. The projection requires momentum systems and a Rhie-Chow user object.

## Pressure pinning

A closed domain has no pressure boundary condition, so the reduced pressure is determined only up
to a constant. Set `pin_pressure = true` with a `pressure_pin_point` and `pressure_pin_value` to
fix it.

## Example Input Syntax

In this example, the executioner solves a collapsing water column. Each time step iterates the
volume fraction, momentum and reduced pressure until the residuals and the largest volume-fraction
change fall below `1e-6`, with at most 30 outer iterations, two PISO correctors per outer iteration, one volume-fraction
subcycle and iterative linear solvers.

!listing modules/navier_stokes/examples/dam_break/dam_break.i block=Executioner

!syntax parameters /Executioner/ReducedPressurePIMPLE

!syntax inputs /Executioner/ReducedPressurePIMPLE

!syntax children /Executioner/ReducedPressurePIMPLE
