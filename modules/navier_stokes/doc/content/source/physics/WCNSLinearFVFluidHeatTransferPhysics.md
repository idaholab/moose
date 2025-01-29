# Navier Stokes Fluid Heat Transfer / WCNSLinearFVFluidHeatTransferPhysics

!syntax description /Physics/NavierStokes/FluidHeatTransferSegregated/WCNSLinearFVFluidHeatTransferPhysics

## Equation

This [Physics](Physics/index.md) object creates the kernels and boundary conditions to solve the advection-diffusion
equation for the fluid temperature.
For free flow in a non-porous media:

!equation
\dfrac{\partial \rho h}{\partial t} + \nabla \cdot (\rho h \mathbf{v}) - \nabla \cdot (k_f \nabla T_f) - Q + \alpha (T_f - T_{ambient}) = 0

!alert note
Porous medium treatment is not implemented for the linear finite volume discretization yet.

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
- [LinearFVVolumetricHeatTransfer.md] for the volumetric ambient convection term, if present


## Automatically defined variables

The `WCNSLinearFVFluidHeatTransferPhysics` automatically sets up the variables which are
necessary for solving the energy transport equation:

- Fluid temperature:

  !listing include/base/NS.h line=std::string T_fluid

For the default names of other variables used in this action, visit [this site](include/base/NS.h).

## Automatically defined functors / materials

The following functor materials are defined:

- [INSFVEnthalpyFunctorMaterial.md] to define functors to compute the specific enthalpy and its time derivative


## Coupling with other Physics

The enthalpy advection equation can be solved concurrently with the flow equations by combining both the `WCNSLinearFVFluidHeatTransferPhysics`
and the [WCNSLinearFVFlowPhysics.md] using the [!param](/Physics/NavierStokes/FluidHeatTransferSegregated/WCNSLinearFVFluidHeatTransferPhysics/coupled_flow_physics) parameter.

!syntax parameters /Physics/NavierStokes/FluidHeatTransferSegregated/WCNSLinearFVFluidHeatTransferPhysics

!syntax inputs /Physics/NavierStokes/FluidHeatTransferSegregated/WCNSLinearFVFluidHeatTransferPhysics

!syntax children /Physics/NavierStokes/FluidHeatTransferSegregated/WCNSLinearFVFluidHeatTransferPhysics
