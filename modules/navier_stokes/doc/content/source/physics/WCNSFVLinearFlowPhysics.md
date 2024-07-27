# Navier Stokes Flow / WCNSFVLinearFlowPhysics

!syntax description /Physics/NavierStokes/FlowLinear/WCNSFVLinearFlowPhysics

## Equations

This [Physics](Physics/index.md) object creates the kernels and boundary conditions to solve the pressure
correction equation and momentum Navier Stokes equations for the flow.
For regular flow in a non-porous medium:

!equation
\nabla \cdot \rho \vec{v} = 0

!equation
\nabla \cdot (\rho \mathbf{v} \otimes \mathbf{v}) = \nabla \cdot (\mu \nabla \mathbf{v}) - \nabla p + \mathbf{F}_g

where:

- $\rho$ is the density
- $\mu$ is the dynamic viscosity
- $\mathbf{v}$ is the velocity (non-porous flow)
- $p$ is the pressure
- $\mathbf{F}_g$ is the gravity term

The kernels created for the momentum equation for free flow:

- [LinearWCNSFVMomentumFlux.md] for the momentum advection and diffusion terms
- [LinearFVMomentumPressure.md] for the pressure gradient term
- [LinearFVSource.md] for the gravity term if specified


The kernels created for free flow for the pressure correction equation:

- [LinearFVAnisotropicDiffusion.md] for the pressure diffusion term
- [LinearFVDivergencen.md] for the pressure divergence term


## Automatically defined variables

The `WCNSFVLinearFlowPhysics` automatically sets up the variables which are
necessary for the solution of a given problem. These variables can then be used
to couple fluid flow simulations with other physics. The list of variable names
commonly used in the action syntax is presented below:

- Velocities for non-porous-medium simulations:

  !listing include/base/NS.h start=std::string velocity_x end=std::string velocity_z include-end=true

- Pressure:

  !listing include/base/NS.h line=pressure

For the default names of other variables used in this action, visit [this site](include/base/NS.h).


## Coupling with other Physics

Coupling with other Physics has not been implemented for `WCNSFVLinearFlowPhysics`.
Coupling can only be performed at the moment by leveraging [MultiApps](syntax/MultiApps/index.md).

!syntax parameters /Physics/NavierStokes/FlowLinear/WCNSFVLinearFlowPhysics

!syntax inputs /Physics/NavierStokes/FlowLinear/WCNSFVLinearFlowPhysics

!syntax children /Physics/NavierStokes/FlowLinear/WCNSFVLinearFlowPhysics
