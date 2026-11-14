# ConservativeDiffuseInterfaceRhieChowMassFlux

!syntax description /UserObjects/ConservativeDiffuseInterfaceRhieChowMassFlux

## Description

`ConservativeDiffuseInterfaceRhieChowMassFlux` is the face-flux provider of the reduced-pressure
diffuse-interface two-phase solve. It builds on [RhieChowMassFlux](RhieChowMassFlux.md), which
documents the inherited fields and parameters, and changes how the pressure, body forces and
transported mass flux enter the segregated solve. The algorithm is described in detail in the
[diffuse-interface dam break algorithm](diffuse_interface_dam_break_algorithm.md) note.

### Reduced pressure and body forces

The pressure variable is the reduced pressure
$p_{rgh} = p - \rho \, \mathbf{g} \cdot (\mathbf{x} - \mathbf{x}_{ref})$, with the
[!param](/UserObjects/ConservativeDiffuseInterfaceRhieChowMassFlux/gravity) vector $\mathbf{g}$
and the
[!param](/UserObjects/ConservativeDiffuseInterfaceRhieChowMassFlux/reference_pressure_point)
$\mathbf{x}_{ref}$. The momentum predictor is assembled without the pressure gradient, gravity
and surface tension. This object adds them explicitly as the force density

\begin{equation}
\mathbf{f} = -\nabla p_{rgh}
  - \left( \mathbf{g} \cdot (\mathbf{x} - \mathbf{x}_{ref}) \right) \nabla \rho
  + \mathbf{f}_{st},
\end{equation}

reconstructed in each cell from its face-normal components on every face, boundary faces
included. The same face-normal forces enter the pressure equation as face fluxes, so the predictor
force, the pressure-equation flux and the corrected velocity are built from the same face
quantities. $\mathbf{f}_{st}$ is
the surface tension force density of
[!param](/UserObjects/ConservativeDiffuseInterfaceRhieChowMassFlux/surface_tension_force_density_functor),
for example from a
[ConservativeDiffuseInterfaceSurfaceTensionFunctorMaterial](ConservativeDiffuseInterfaceSurfaceTensionFunctorMaterial.md);
without it there is no capillary force. The pressure projection is always the standard one: the
consistent (SIMPLEC) projection would replace the unrelaxed, diagonally dominant $1/A$ with a row
sum of the relaxed matrix.

### Transported fluxes

The face flux that advances the volume fraction and the face mass flux that advects momentum are
kept consistent:

- `corrected_face_phi` is the pressure-corrected volumetric face flux, and
  `lagged_corrected_face_phi` is that flux frozen once per outer iteration. The volume fraction
  and the outlet boundary conditions read the frozen flux, so all of them see the same flux within
  an iteration.
- The face mass flux used by momentum advection is the `rho_phi` flux published by the
  volume-fraction transport, named by
  [!param](/UserObjects/ConservativeDiffuseInterfaceRhieChowMassFlux/vof_rho_phi_functor).
  It is required: across a density jump the transported mass flux is not the product of one face
  density and one volumetric flux.

The object also provides the maximum cell Courant number of the corrected flux, which the
[ReducedPressurePIMPLE](ReducedPressurePIMPLE.md) executioner uses to size its volume-fraction
subcycles. Face fluxes are exchanged to halo faces in parallel. The transport history is
restartable, so a step can be repeated or recovered; it includes the boundary pressure gradient
and face $1/A$ of the accepted step, which the first momentum predictor of the next step reads.

This object is created by
[WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics](WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics.md),
which passes on its gravity, reference point and, when surface tension is
enabled, the generated surface tension force density.

## Example Input Syntax

In this example, the flow physics `flow` creates this object for a flat interface between two
fluids at rest under gravity, where the reduced-pressure coupling must cancel the buoyancy source
and leave the fluid at rest.

!listing modules/navier_stokes/test/tests/finite_volume/two_phase/diffuse_interface/hydrostatic_interface.i block=Physics/NavierStokes/ConservativeDiffuseInterfaceFlowSegregated

!syntax parameters /UserObjects/ConservativeDiffuseInterfaceRhieChowMassFlux

!syntax inputs /UserObjects/ConservativeDiffuseInterfaceRhieChowMassFlux

!syntax children /UserObjects/ConservativeDiffuseInterfaceRhieChowMassFlux
