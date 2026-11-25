# WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics

!syntax description /Physics/NavierStokes/ConservativeDiffuseInterfaceFlowSegregated/WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics

## Overview

`WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics` sets up the momentum and pressure equations
of a two-phase diffuse-interface flow, solved with linear finite volume variables by the
[ReducedPressurePIMPLE](ReducedPressurePIMPLE.md) executioner. It is used together with a
[WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics](WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics.md)
that transports the volume fraction and defines the mixture density and viscosity. The algorithm
is described in the
[diffuse-interface dam break algorithm](diffuse_interface_dam_break_algorithm.md) note.

It extends [WCNSLinearFVFlowPhysics](WCNSLinearFVFlowPhysics.md) and differs from it as follows:

- The pressure variable is the reduced pressure $p_{rgh} = p - \rho\, \mathbf{g} \cdot
  \left( \mathbf{x} - \mathbf{x}_{ref} \right)$, so `solve_for_dynamic_pressure` must be `true`,
  which is the default here.
- The flow coupling is a
  [ConservativeDiffuseInterfaceRhieChowMassFlux](ConservativeDiffuseInterfaceRhieChowMassFlux.md).
  It assembles the momentum predictor without the pressure, gravity and capillary forces and
  carries them into the pressure equation as face fluxes. Momentum is advected with the face mass
  flux of the limited volume-fraction update, `vof_rho_phi_functor`. A different Rhie-Chow object
  on the same blocks is an error.
- The pressure equation is a
  [LinearFVPressureCorrectionDiffusion](LinearFVPressureCorrectionDiffusion.md) with the face
  $1/A$ published by the Rhie-Chow object, and a [LinearFVDivergence](LinearFVDivergence.md) of
  the predictor flux less the explicit face forces.
- Gravity acts through the face-normal gradient of the density.

## Boundary conditions

- `noslip` walls get a [LinearFVAdvectionDiffusionFunctorDirichletBC](LinearFVAdvectionDiffusionFunctorDirichletBC.md)
  on each velocity component, with the wall velocity functors when given and zero otherwise, and
  a [LinearFVPressureFluxBC](LinearFVPressureFluxBC.md) that imposes the pressure gradient which
  makes the wall flux zero. `symmetry` walls get
  [LinearFVVelocitySymmetryBC](LinearFVVelocitySymmetryBC.md) and
  [LinearFVPressureSymmetryBC](LinearFVPressureSymmetryBC.md). Other wall types are not supported.
  `pressure_two_term_bc_expansion` is ignored on walls.
- `fixed-pressure` and `fixed-pressure-zero-gradient` outlets get a
  [LinearFVPressureInletOutletVelocityBC](LinearFVPressureInletOutletVelocityBC.md) on each
  velocity component and a [LinearFVPrghTotalPressureBC](LinearFVPrghTotalPressureBC.md) on the
  reduced pressure, so the flow may reverse through them. `pressure_functors` needs one entry per
  such outlet. `zero-gradient` outlets get a
  [LinearFVAdvectionDiffusionOutflowBC](LinearFVAdvectionDiffusionOutflowBC.md) on the velocity.

## Interface geometry and surface tension

With
[!param](/Physics/NavierStokes/ConservativeDiffuseInterfaceFlowSegregated/WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics/create_geometry_functors)
`= true` and a
[!param](/Physics/NavierStokes/ConservativeDiffuseInterfaceFlowSegregated/WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics/volume_fraction_functor),
the physics adds a
[ConservativeDiffuseInterfaceGeometryFunctorMaterial](ConservativeDiffuseInterfaceGeometryFunctorMaterial.md)
that defines the face interface unit normal `<physics name>_interface_unit_normal_face`, with the
regularization
[!param](/Physics/NavierStokes/ConservativeDiffuseInterfaceFlowSegregated/WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics/geometry_delta_n).
The volume-fraction physics reads this normal for its compression flux. Without a volume-fraction
functor, the material is not created and a warning is issued.

A nonempty
[!param](/Physics/NavierStokes/ConservativeDiffuseInterfaceFlowSegregated/WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics/surface_tension_coefficient)
adds a
[ConservativeDiffuseInterfaceSurfaceTensionFunctorMaterial](ConservativeDiffuseInterfaceSurfaceTensionFunctorMaterial.md)
and passes its capillary force density to the Rhie-Chow object. Surface tension requires the
volume-fraction functor. It also widens the algebraic ghosting from two to three layers of cells,
since the curvature is the divergence of a normal that is itself a gradient.

## Example Input Syntax

In this example, the flow physics sets up a collapsing water column in air. The density and
viscosity are the mixture functors of the volume-fraction physics, the bottom and sides are no-slip
walls, and the top is a `fixed-pressure` outlet through which air may flow in and out. The interface
normal is built from the volume fraction `alpha`.

!listing modules/navier_stokes/examples/dam_break/dam_break.i block=Physics/NavierStokes/ConservativeDiffuseInterfaceFlowSegregated

!syntax parameters /Physics/NavierStokes/ConservativeDiffuseInterfaceFlowSegregated/WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics

!syntax inputs /Physics/NavierStokes/ConservativeDiffuseInterfaceFlowSegregated/WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics

!syntax children /Physics/NavierStokes/ConservativeDiffuseInterfaceFlowSegregated/WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics
