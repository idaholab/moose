# WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics

!syntax description /Physics/NavierStokes/ConservativeDiffuseInterfaceVOFSegregated/WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics

## Overview

`WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics` transports the liquid volume fraction
$\alpha$ of a two-phase diffuse-interface flow and defines the mixture properties the flow reads.
It is coupled, through `coupled_flow_physics`, to a
[WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics](WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics.md),
and both are solved by the [ReducedPressurePIMPLE](ReducedPressurePIMPLE.md) executioner. The
algorithm is described in the
[diffuse-interface dam break algorithm](diffuse_interface_dam_break_algorithm.md) note.

The volume fraction obeys

\begin{equation}
\frac{\partial \alpha}{\partial t} + \nabla \cdot \left( \alpha \mathbf{u} \right)
+ \nabla \cdot \left( c_\alpha \left| \mathbf{u} \right| \alpha (1 - \alpha) \mathbf{n} \right) = 0,
\end{equation}

where the last term is an interface compression flux along the interface unit normal
$\mathbf{n}$, scaled by the
[!param](/Physics/NavierStokes/ConservativeDiffuseInterfaceVOFSegregated/WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics/compression_factor)
$c_\alpha$. Each update is an implicit upwind solve followed by an explicit bounded correction
toward a Van Leer face value plus the compression flux, limited so that $\alpha$ stays within the
bounds of its neighbors.

## Created objects

- A [LinearFVScalarAdvection](LinearFVScalarAdvection.md) kernel for the upwind solve, advected
  with the face flux frozen for the current outer iteration, `lagged_corrected_face_phi`.
- A [ConservativeDiffuseInterfaceVOFMULESCorrector](ConservativeDiffuseInterfaceVOFMULESCorrector.md)
  that applies the bounded correction, with
  [!param](/Physics/NavierStokes/ConservativeDiffuseInterfaceVOFSegregated/WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics/n_alpha_corrections)
  correction sweeps of
  [!param](/Physics/NavierStokes/ConservativeDiffuseInterfaceVOFSegregated/WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics/n_limiter_iterations)
  limiter passes each, and the compression along
  [!param](/Physics/NavierStokes/ConservativeDiffuseInterfaceVOFSegregated/WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics/interface_normal_functor).
  It publishes the limited volume-fraction flux `alpha_phi_limited` and the mass flux `rho_phi`
  that the flow advects momentum with.
- A [LinearFVInletOutletScalarBC](LinearFVInletOutletScalarBC.md) on every outlet of the coupled
  flow. Fluid flowing back in carries
  [!param](/Physics/NavierStokes/ConservativeDiffuseInterfaceVOFSegregated/WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics/volume_fraction_outlet_backflow_functor),
  zero by default, which suits air entering through an open top.
  [!param](/Physics/NavierStokes/ConservativeDiffuseInterfaceVOFSegregated/WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics/volume_fraction_two_term_bc_expansion)
  sets whether outflow extrapolates with the cell gradient. The boundary condition carries the
  cell value, and the corrector adds the extrapolation as a limited correction.
- The mixture density
  [!param](/Physics/NavierStokes/ConservativeDiffuseInterfaceVOFSegregated/WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics/mixture_density_name)
  and viscosity
  [!param](/Physics/NavierStokes/ConservativeDiffuseInterfaceVOFSegregated/WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics/mixture_dynamic_viscosity_name),
  both $\alpha \phi_l + (1 - \alpha) \phi_g$ of the phase values
  [!param](/Physics/NavierStokes/ConservativeDiffuseInterfaceVOFSegregated/WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics/liquid_density_name),
  [!param](/Physics/NavierStokes/ConservativeDiffuseInterfaceVOFSegregated/WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics/gas_density_name),
  [!param](/Physics/NavierStokes/ConservativeDiffuseInterfaceVOFSegregated/WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics/liquid_dynamic_viscosity_name)
  and
  [!param](/Physics/NavierStokes/ConservativeDiffuseInterfaceVOFSegregated/WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics/gas_dynamic_viscosity_name).
  Only the viscosity clips $\alpha$ to $[0, 1]$, so the density stays exactly linear in $\alpha$.
  The coupled flow physics should use these names as its density and viscosity.
- The complementary volume fraction $1 - \alpha$, named
  [!param](/Physics/NavierStokes/ConservativeDiffuseInterfaceVOFSegregated/WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics/complementary_volume_fraction_name),
  as a [ParsedFunctorMaterial](ParsedFunctorMaterial.md) output to all outputs, unless a functor
  of that name already exists.
- The initial condition of $\alpha$ from
  [!param](/Physics/NavierStokes/ConservativeDiffuseInterfaceVOFSegregated/WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics/initial_scalar_variables).

## Restrictions

- Exactly one volume fraction is transported:
  [!param](/Physics/NavierStokes/ConservativeDiffuseInterfaceVOFSegregated/WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics/passive_scalar_names)
  must have one entry, and the complementary name must differ from it.
- [!param](/Physics/NavierStokes/ConservativeDiffuseInterfaceVOFSegregated/WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics/passive_scalar_advection_interpolation)
  must be `upwind`: the high-order part of the flux comes only from the bounded correction.
- When the volume-fraction variable is defined outside this physics, its initial condition must be
  defined there too.

## Example Input Syntax

In this example, the physics transports the water fraction `alpha` of a collapsing water column.
It defines `rho_mixture` and `mu_mixture` from the water and air properties for the flow physics
`flow`, compresses the interface along the normal that `flow` generates, and applies one bounded
correction sweep of up to six limiter passes per update.

!listing modules/navier_stokes/examples/dam_break/dam_break.i block=Physics/NavierStokes/ConservativeDiffuseInterfaceVOFSegregated

!syntax parameters /Physics/NavierStokes/ConservativeDiffuseInterfaceVOFSegregated/WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics

!syntax inputs /Physics/NavierStokes/ConservativeDiffuseInterfaceVOFSegregated/WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics

!syntax children /Physics/NavierStokes/ConservativeDiffuseInterfaceVOFSegregated/WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics
