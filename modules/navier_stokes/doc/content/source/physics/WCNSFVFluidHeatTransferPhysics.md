# Navier Stokes Fluid Heat Transfer / WCNSFVFluidHeatTransferPhysics

!syntax description /Physics/NavierStokes/FluidHeatTransfer/WCNSFVFluidHeatTransferPhysics

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

!tag name=WCNSFVFluidHeatTransferPhysics pairs=module:navier_stokes system:physics
