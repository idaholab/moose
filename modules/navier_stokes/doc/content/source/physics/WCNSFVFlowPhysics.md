# WCNSFVFlowPhysics

!syntax description /Physics/NavierStokes/WCNSFVFlow/WCNSFVFlowPhysics

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

!syntax parameters /Physics/NavierStokes/WCNSFVFlow/WCNSFVFlowPhysics

!syntax inputs /Physics/NavierStokes/WCNSFVFlow/WCNSFVFlowPhysics

!syntax children /Physics/NavierStokes/WCNSFVFlow/WCNSFVFlowPhysics
