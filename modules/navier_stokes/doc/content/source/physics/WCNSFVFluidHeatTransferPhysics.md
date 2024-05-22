# Navier Stokes Fluid Heat Transfer / WCNSFVFluidHeatTransferPhysics

!syntax description /Physics/NavierStokes/FluidHeatTransfer/WCNSFVFluidHeatTransferPhysics

## Equation

This [Physics](Physics/index.md) object creates the kernels and boundary conditions to solve the advection-diffusion
equation for the fluid temperature.
For free flow in a non-porous media:

!equation
\dfrac{\partial \rho h}{\partial t} + \nabla \cdot (\rho h \mathbf{v}) - \nabla \cdot (k_f \nabla T_f) - Q + \alpha (T_f - T_{ambient}) = 0

For flow in a porous medium:

!equation
\dfrac{\partial \epsilon \rho h}{\partial t} + \nabla \cdot (\rho h \mathbf{v}_D) - \nabla \cdot (k_f \nabla T_f) - Q + \alpha (T_f - T_{ambient}) = 0

where:

- $h$ is the fluid enthalpy, computed from the specific heat $c_p$
- $\rho$ is the fluid density
- $\epsilon$ is the porosity
- $T_f$ is the fluid temperature
- \mathbf{v} is the advecting velocity (clean flow)
- \mathbf{v}_D is the advecting superficial velocity (porous media flow)
- $kappa_f$ the fluid effective thermal conductivity
- $Q$ is the source term, corresponding to energy deposited directly in the fluid
- $\alpha$ is the ambient convection volumetric heat transfer coefficient
- $T_{ambient}$ is the ambient temperature

The enthalpy is used in lieu of $\rho c_p T$ to be able to model gases with temperature dependent
specific heat.

The kernels created for flow in a non-porous medium are:

- [INSFVEnergyTimeDerivative.md] for the time derivative for incompressible flow a transient solve
- [WCNSFVEnergyTimeDerivative.md] for the time derivative for weakly-compressible flow a transient solve
- [INSFVEnergyAdvection.md] for advection
- [FVDiffusion.md] for diffusion
- [FVCoupledForce.md] for the energy source term
- [PINSFVEnergyAmbientConvection.md] for the volumetric ambient convection term, if present

For flow in a porous medium:

- [PINSFVEnergyTimeDerivative.md] for the time derivative in a transient solve
- [PINSFVEnergyAdvection.md] for energy advection
- [PINSFVEnergyDiffusion.md] for energy diffusion with an isotropic thermal diffusivity
- [PINSFVEnergyAnisotropicDiffusion.md] for energy diffusion with an anisotropic thermal diffusivity
- [FVCoupledForce.md] for the energy source term
- [PINSFVEnergyAmbientConvection.md] for the volumetric ambient convection term, if present

!alert note
Additional details on porous media flow equations can be found on this [page](navier_stokes/pinsfv.md).

## Automatically defined variables

The `WCNSFVFluidHeatTransferPhysics` automatically sets up the variables which are
necessary for solving the energy transport equation:

- Fluid temperature:

  !listing include/base/NS.h line=std::string T_fluid

For the default names of other variables used in this action, visit [this site](include/base/NS.h).


## Coupling with other Physics

The heat advection equation can be solved concurrently with the flow equations by combining both the [WCNSFVFluidHeatTransferPhysics.md]
and the [WCNSFVFlowPhysics.md].
The following input performs this coupling for incompressible flow in a 2D channel.
No system parameters are passed, so the equations are solved in a fully coupled manner in the same [nonlinear system](systems/NonlinearSystem.md).

!listing test/tests/finite_volume/ins/channel-flow/2d-rc-transient-physics.i block=Physics

!syntax parameters /Physics/NavierStokes/FluidHeatTransfer/WCNSFVFluidHeatTransferPhysics

!syntax inputs /Physics/NavierStokes/FluidHeatTransfer/WCNSFVFluidHeatTransferPhysics

!syntax children /Physics/NavierStokes/FluidHeatTransfer/WCNSFVFluidHeatTransferPhysics
