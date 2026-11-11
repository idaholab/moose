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

## Porous medium flow

With [!param](/Physics/NavierStokes/FlowSegregated/WCNSLinearFVFlowPhysics/porous_medium_treatment)
set to `true`, the Physics solves for the superficial velocity
$\mathbf{U} = \epsilon \mathbf{v}$, where $\epsilon$ is the
[!param](/Physics/NavierStokes/FlowSegregated/WCNSLinearFVFlowPhysics/porosity):

!equation
\nabla \cdot \rho \mathbf{U} = 0

!equation
\nabla \cdot \left(\dfrac{\rho}{\epsilon} \mathbf{U} \otimes \mathbf{U}\right) = \nabla \cdot \left(\mu \left[\nabla \mathbf{v}+\nabla \mathbf{v}^T\right]\right) - \epsilon \nabla p - \mathbf{W}\mathbf{U} + \epsilon \mathbf{F}_g

where $\mathbf{W}$ is the friction tensor of the Darcy and Forchheimer terms. The objects created
in place of their free-flow counterparts are:

- [PorousRhieChowMassFlux.md] for the face mass fluxes
- [LinearPWCNSFVMomentumFlux.md] for the momentum advection and diffusion terms
- [LinearPWCNSFVMomentumPressure.md] for the porosity-weighted pressure gradient
- [LinearFVMomentumFriction.md] for the Darcy and Forchheimer friction, listed in
  [!param](/Physics/NavierStokes/FlowSegregated/WCNSLinearFVFlowPhysics/friction_types)
- [LinearFVPressureFluxBC.md] for the pressure on the walls, which uses the wall velocity and the
  $A^{-1}H$ flux

The gravity term, the [LinearFVMomentumBoussinesq.md] term and the friction terms are weighted by
the same porosity.

### Pressure-jump baffles

A sharp porosity change is best represented by a pressure-jump baffle on an internal sideset.
Setting
[!param](/Physics/NavierStokes/FlowSegregated/WCNSLinearFVFlowPhysics/porosity_interface_pressure_treatment)
to `bernoulli` places a [BernoulliFormLossPressureJump.md] model on the sidesets listed in
[!param](/Physics/NavierStokes/FlowSegregated/WCNSLinearFVFlowPhysics/pressure_drop_sidesets).
The optional irreversible form loss of each sideset is given by
[!param](/Physics/NavierStokes/FlowSegregated/WCNSLinearFVFlowPhysics/pressure_drop_form_factors),
and the side of the baffle providing its reference velocity by
[!param](/Physics/NavierStokes/FlowSegregated/WCNSLinearFVFlowPhysics/pressure_drop_reference_velocity_side).
With baffles, the Physics also creates:

- [LinearFVPressureCorrectionDiffusionJump.md] for the pressure correction diffusion term
- [FVPressureJumpGreenGaussGradient.md] as the gradient method of the pressure variable
- [FVVelocityJumpGreenGaussGradient.md] as the gradient method of the velocity variables
- [FVReconstructedPressureGradient.md] for the pressure gradient of the momentum equations, unless
  [!param](/Physics/NavierStokes/FlowSegregated/WCNSLinearFVFlowPhysics/use_reconstructed_pressure_gradient)
  is `false`. Its relaxation is set by
  [!param](/Physics/NavierStokes/FlowSegregated/WCNSLinearFVFlowPhysics/pressure_gradient_relaxation).

The pressure jumps are relaxed with
[!param](/Physics/NavierStokes/FlowSegregated/WCNSLinearFVFlowPhysics/pressure_jump_relaxation).
The Rhie-Chow user object is named `pins_rhie_chow_interpolator` by default for porous flow, which
the [SIMPLE.md] executioner must reference.

The following input defines a one-dimensional porous channel with two baffles and Forchheimer
friction in the porous block:

!listing test/tests/finite_volume/pins/channel-flow/linear-segregated/1d-simple-channel/porous-baffle-1d-physics.i block=Physics

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

- Velocities for porous medium simulations:

  !listing include/base/NS.h start=std::string superficial_velocity_x end=std::string superficial_velocity_z include-end=true

- Pressure:

  !listing include/base/NS.h line=pressure

For the default names of other variables used in this action, visit [!file text=this site](include/base/NS.h).


## Coupling with other Physics

The [WCNSLinearFVFluidHeatTransferPhysics.md], [WCNSLinearFVScalarTransportPhysics.md],
[WCNSLinearFVTurbulencePhysics.md] and [WCNSLinearFVTwoPhaseMixturePhysics.md] couple to the face
mass fluxes of the `WCNSLinearFVFlowPhysics`. The turbulence and two-phase mixture Physics do not
support the porous medium treatment.

!syntax parameters /Physics/NavierStokes/FlowSegregated/WCNSLinearFVFlowPhysics

!syntax inputs /Physics/NavierStokes/FlowSegregated/WCNSLinearFVFlowPhysics

!syntax children /Physics/NavierStokes/FlowSegregated/WCNSLinearFVFlowPhysics
