# Navier Stokes Flow / WCNSFVFlowPhysics

!syntax description /Physics/NavierStokes/Flow/WCNSFVFlowPhysics

## Equations

This [Physics](Physics/index.md) object creates the kernels and boundary conditions to solve the mass and momentum Navier Stokes equations for the flow.
The time derivatives in the mass equations are omitted for incompressible flow.
For regular flow in a non-porous medium:

!equation
\dfrac{\partial \rho}{\partial t} + \nabla \cdot \rho \vec{v} = 0

!equation
\dfrac{\partial \rho \mathbf{v}}{\partial t} + \nabla \cdot (\rho \mathbf{v} \otimes \mathbf{v}) = \nabla \cdot (\mu \nabla \mathbf{v}) - \nabla p + (\mathbf{F}_g + \mathbf{F}_f)

For porous media flow:

!equation
\epsilon \dfrac{\partial \rho}{\partial t} + \nabla \cdot \rho \vec{v}_D = 0

!equation
\dfrac{\partial \rho \mathbf{v}_D}{\partial t} + \nabla \cdot (\dfrac{\rho}{\epsilon} \mathbf{v}_D \otimes \mathbf{v}_D) = \nabla \cdot (\mu \nabla \dfrac{\mathbf{v}_D}{\epsilon}) - \epsilon \nabla p + \epsilon (\mathbf{F}_g + \mathbf{F}_f)

where:

- $\rho$ is the density
- $\mu$ is the dynamic viscosity
- $\epsilon$ is the porosity
- $\mathbf{v}$ is the velocity (non-porous flow)
- $\mathbf{v}_D$ is the superficial velocity (porous flow)
- $p$ is the pressure
- $\mathbf{F}_g$ is the gravity term
- $\mathbf{F}_f$ is the friction / inter-phase friction term


!alert note
Additional details on porous media flow equations can be found on this [page](navier_stokes/pinsfv.md).

The kernels created for free flow for the mass equation:

- [WCNSFVMassTimeDerivative.md] for weakly-compressible flow in a transient case
- [INSFVMassAdvection.md] for the mass advection term

for porous media flow:

- [PWCNSFVMassTimeDerivative.md] for weakly-compressible flow
- [PINSFVMassAdvection.md] for mass advection

The kernels created for the momentum equation for free flow:

- [WCNSFVMomentumTimeDerivative.md] for the time derivative for weakly-compressible flow in a transient case
- [INSFVMomentumTimeDerivative.md] for the time derivative incompressible flow in a transient case
- [INSFVMomentumAdvection.md] for the momentum advection term
- [INSFVMomentumDiffusion.md] for the momentum diffusion term
- [INSFVMomentumPressure.md] for the pressure gradient term
- [PINSFVMomentumFriction.md] for the friction term if specified
- [INSFVMomentumGravity.md] for the gravity term if specified

for porous media flow:

- [PINSFVMomentumTimeDerivative.md] for the time derivative
- [PINSFVMomentumAdvection.md] for the momentum advection term
- [PINSFVMomentumDiffusion.md] for the momentum diffusion term
- [PINSFVMomentumPressure.md] for the pressure gradient term
- [PINSFVMomentumFriction.md] for the friction term if specified
- [PINSFVMomentumGravity.md] for the gravity term if specified

## Automatically defined variables

The `WCNSFVFlowPhysics` automatically sets up the variables which are
necessary for the solution of a given problem. These variables can then be used
to couple fluid flow simulations with other physics. The list of variable names
commonly used in the action syntax is presented below:

- Velocities for non-porous-medium simulations:

  !listing include/base/NS.h start=std::string velocity_x end=std::string velocity_z include-end=true

- Velocities for porous medium simulations:

  !listing include/base/NS.h start=std::string superficial_velocity_x end=std::string superficial_velocity_z include-end=true

- Pressure:

  !listing include/base/NS.h line=pressure

For the default names of other variables used in this action, visit [this site](include/base/NS.h).


## Coupling with other Physics

The energy advection equation can be solved concurrently with the flow equations using an additional [WCNSFVFluidHeatTransferPhysics.md].
The following input performs this coupling for incompressible flow in a 2D flow channel.
No system parameters are passed, so the equations are solved in a fully coupled manner in the same [nonlinear system](systems/NonlinearSystem.md).

!listing test/tests/finite_volume/ins/channel-flow/2d-rc-transient-physics.i block=Physics

Other advected scalar equations can be solved concurrently with the flow equations using an additional [WCNSFVScalarTransportPhysics.md].
The following input performs this coupling for incompressible flow in a 2D flow channel.
No system parameters are passed, so the equations are solved in a fully coupled manner in the same [nonlinear system](systems/NonlinearSystem.md).

!listing test/tests/finite_volume/ins/channel-flow/2d-scalar-transport-physics.i block=Physics

## Modeling options and implementation details

!alert! note

This physics only supports Rhie-Chow interpolation for the determination
of face velocities in the advection terms. The face interpolation of the
advected quantities (e.g. upwind, average) can be controlled through the
`*_advection_interpolation` physics parameters.

!alert-end!

#### Bernoulli pressure jump treatment

Please see [the Bernoulli pressure variable documentation](BernoulliPressureVariable.md) for more information.

!syntax parameters /Physics/NavierStokes/Flow/WCNSFVFlowPhysics

!syntax inputs /Physics/NavierStokes/Flow/WCNSFVFlowPhysics

!syntax children /Physics/NavierStokes/Flow/WCNSFVFlowPhysics
