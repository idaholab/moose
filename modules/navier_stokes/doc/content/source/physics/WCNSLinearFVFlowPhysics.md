# Navier Stokes Flow Segregated / WCNSLinearFVFlowPhysics

!syntax description /Physics/NavierStokes/FlowSegregated/WCNSLinearFVFlowPhysics

## Equations

This [Physics](Physics/index.md) object creates the kernels and boundary conditions to solve the Navier Stokes equations for the flow
using the [SIMPLE.md] algorithm.
For regular flow in a non-porous medium:

!equation
\nabla \cdot \rho \vec{v} = 0

!equation
\nabla \cdot (\rho \mathbf{v} \otimes \mathbf{v}) = \nabla \cdot \left(\mu \left[\nabla \mathbf{v}+\nabla \mathbf{v}^T\right]\right) - \nabla p + \mathbf{F}_g

where:

- $\rho$ is the density
- $\mu$ is the dynamic viscosity
- $\mathbf{v}$ is the velocity (non-porous flow)
- $p$ is the pressure
- $\mathbf{F}_g$ is the gravitational force

The kernels created for the momentum equation for free flow:

- [LinearWCNSFVMomentumFlux.md] for the momentum advection and diffusion terms
- [LinearFVMomentumPressure.md] for the pressure gradient term
- [LinearFVSource.md] for the gravity term if specified


The kernels created for free flow for the pressure correction equation:

- [LinearFVPressureCorrectionDiffusion.md] for the pressure diffusion term
- [LinearFVDivergence.md] for the divergence of $A^{-1}H$. For more information, see [SIMPLE.md].

## Selecting interpolation methods

The linear finite volume Physics syntax provides shortcut parameters for the interpolation methods
used by the kernels it creates. The
[!param](/Physics/NavierStokes/FlowSegregated/WCNSLinearFVFlowPhysics/momentum_advection_interpolation)
parameter selects the momentum advection interpolation used by [LinearWCNSFVMomentumFlux.md].
The supported choices are `average`, `upwind`, `vanLeer`, `min_mod`, and `venkatakrishnan`.

The
[!param](/Physics/NavierStokes/FlowSegregated/WCNSLinearFVFlowPhysics/pressure_diffusion_interpolation)
parameter selects the interpolation used to compute the face values of `Ainv`, the $A^{-1}$
diffusion tensor in the pressure correction diffusion term. The supported choices are `average`
and `harmonic`.

Use these shortcut parameters directly in the Physics block. For direct linear finite volume
advection kernel syntax, add the interpolation methods in `[FVInterpolationMethods]` and pass
their names to the kernels. For direct pressure correction syntax, set
`pressure_diffusion_interpolation` on [RhieChowMassFlux.md].

!listing modules/navier_stokes/test/tests/finite_volume/ins/lid-driven/linear-segregated/lid-driven-segregated-physics.i line=pressure_diffusion_ainv_interp_method

!listing modules/navier_stokes/test/tests/finite_volume/ins/lid-driven/linear-segregated/lid-driven-segregated-physics.i start=momentum_advection_interpolation end=pressure_diffusion_interpolation include-end=true

## Automatically defined variables

The `WCNSLinearFVFlowPhysics` automatically sets up the variables which are
necessary for the solution of a given problem. These variables can then be used
to couple fluid flow simulations with other physics. The list of variable names
commonly used in the action syntax is presented below:

- Velocities for non-porous-medium simulations:

  !listing include/base/NS.h start=std::string velocity_x end=std::string velocity_z include-end=true

- Pressure:

  !listing include/base/NS.h line=pressure

For the default names of other variables used in this action, visit [this site](include/base/NS.h).


## Coupling with other Physics

Coupling with other Physics has not been implemented for `WCNSLinearFVFlowPhysics`.
Coupling can only be performed at the moment by leveraging [MultiApps](syntax/MultiApps/index.md).

!syntax parameters /Physics/NavierStokes/FlowSegregated/WCNSLinearFVFlowPhysics

!syntax inputs /Physics/NavierStokes/FlowSegregated/WCNSLinearFVFlowPhysics

!syntax children /Physics/NavierStokes/FlowSegregated/WCNSLinearFVFlowPhysics
