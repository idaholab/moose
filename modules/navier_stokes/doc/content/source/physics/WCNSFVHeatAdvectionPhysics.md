# WCNSFVHeatAdvectionPhysics

!syntax description /Physics/NavierStokes/WCNSFVHeatAdvection/WCNSFVHeatAdvectionPhysics

## Automatically defined variables

The `WCNSFVHeatAdvectionPhysics` automatically sets up the variables which are
necessary for solving the heat advection equation:

- Fluid temperature:

  !listing include/base/NS.h line=std::string T_fluid

For the default names of other variables used in this action, visit [this site](include/base/NS.h).


## Coupling with other Physics

The heat advection equation can be solved concurrently with the flow equations by combining both the [WCNSFVHeatAdvectionPhysics.md]
and the [WCNSFVFlowPhysics.md].
The following input performs this coupling for incompressible flow in a 2D channel.
No system parameters are passed, so the equations are solved in a fully coupled manner in the same [nonlinear system](systems/NonlinearSystem.md).

!listing test/tests/finite_volume/ins/channel-flow/2d-rc-transient.i block=Physics

!syntax parameters /Physics/NavierStokes/WCNSFVHeatAdvection/WCNSFVHeatAdvectionPhysics

!syntax inputs /Physics/NavierStokes/WCNSFVHeatAdvection/WCNSFVHeatAdvectionPhysics

!syntax children /Physics/NavierStokes/WCNSFVHeatAdvection/WCNSFVHeatAdvectionPhysics
