# Navier Stokes Fluid Heat Transfer / WCNSLinearFVFluidHeatTransferPhysics

!syntax description /Physics/NavierStokes/FluidHeatTransferSegregated/WCNSLinearFVFluidHeatTransferPhysics

## Equation

This [Physics](Physics/index.md) object creates the kernels and boundary conditions to solve the advection-diffusion
equation for the fluid temperature.
For free flow in a non-porous media:

!equation
\dfrac{\partial \rho h}{\partial t} + \nabla \cdot (\rho h \mathbf{v}) - \nabla \cdot (k_f \nabla T_f) - Q + \alpha (T_f - T_{ambient}) = 0

where:

- $h$ is the fluid specific enthalpy
- $\rho$ is the fluid density
- $T_f$ is the fluid temperature
- \mathbf{v} is the advecting velocity (clean flow)
- $kappa_f$ the fluid effective thermal conductivity
- $Q$ is the source term, corresponding to energy deposited directly in the fluid
- $\alpha$ is the ambient convection volumetric heat transfer coefficient
- $T_{ambient}$ is the ambient temperature

The kernels created for flow in a non-porous medium are:

- [LinearFVEnergyAdvection.md] for advection
- [LinearFVDiffusion.md] for diffusion
- [LinearFVSource.md] for the energy source term
- [LinearFVVolumetricHeatTransfer.md] for the volumetric ambient convection term, if present.
  When solving for enthalpy, [LinearFVEnthalpyVolumetricHeatTransfer.md] is used instead, which
  linearizes the fluid temperature with $dT/dh = 1/c_p$.

## Porous medium flow

When the coupled [WCNSLinearFVFlowPhysics.md] uses the porous medium treatment, the energy
equation becomes

!equation
\dfrac{\partial \epsilon \rho h}{\partial t} + \nabla \cdot (\rho h \mathbf{U}) - \nabla \cdot (\kappa_f \nabla T_f) - Q + \alpha (T_f - T_{ambient}) = 0

where $\mathbf{U}$ is the superficial velocity, so the advection term uses the face mass flux of
the [PorousRhieChowMassFlux.md] without a porosity factor. The effective fluid conductivity
$\kappa_f$ is the given conductivity by default, and $\epsilon k_f$ if
[!param](/Physics/NavierStokes/FluidHeatTransferSegregated/WCNSLinearFVFluidHeatTransferPhysics/effective_conductivity)
is `false`. The source $Q$ and the heat transfer coefficient $\alpha$ are given per unit total
volume. The solid phase is coupled through the ambient convection term with the
[PNSLinearFVSolidHeatTransferPhysics.md].

!listing test/tests/finite_volume/pins/channel-flow/linear-segregated/2d-simple-channel-enthalpy/porous-baffle-2d-enthalpy-physics.i block=Physics


## Automatically defined variables

The `WCNSLinearFVFluidHeatTransferPhysics` automatically sets up the variables which are
necessary for solving the energy transport equation:

- Fluid temperature:

  !listing include/base/NS.h line=std::string T_fluid

For the default names of other variables used in this action, visit [!file text=this site](include/base/NS.h).

## Automatically defined functors / materials

The following functor materials are defined:

- [INSFVEnthalpyFunctorMaterial.md] to define functors to compute the specific enthalpy and its time derivative


## Coupling with other Physics

The enthalpy advection equation can be solved concurrently with the flow equations by combining both the `WCNSLinearFVFluidHeatTransferPhysics`
and the [WCNSLinearFVFlowPhysics.md] using the [!param](/Physics/NavierStokes/FluidHeatTransferSegregated/WCNSLinearFVFluidHeatTransferPhysics/coupled_flow_physics) parameter.

!syntax parameters /Physics/NavierStokes/FluidHeatTransferSegregated/WCNSLinearFVFluidHeatTransferPhysics

!syntax inputs /Physics/NavierStokes/FluidHeatTransferSegregated/WCNSLinearFVFluidHeatTransferPhysics

!syntax children /Physics/NavierStokes/FluidHeatTransferSegregated/WCNSLinearFVFluidHeatTransferPhysics
