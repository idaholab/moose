# WCNSFVScalarAdvectionPhysics

!syntax description /Physics/NavierStokes/WCNSFVScalarAdvection/WCNSFVScalarAdvectionPhysics

## Coupling with other Physics

Scalar avection equations can be solved concurrently with the flow equations by combining the [WCNSFVFlowPhysics.md] with the `WCNSFVScalarAdvectionPhysics`.
The following input performs this coupling for incompressible flow in a 2D channel.
No system parameters are passed, so the equations are solved in a fully coupled manner in the same [nonlinear system](systems/NonlinearSystem.md).

!listing test/tests/finite_volume/ins/channel-flow/2d-scalar-transport-physics.i block=Physics

!syntax parameters /Physics/NavierStokes/WCNSFVScalarAdvection/WCNSFVScalarAdvectionPhysics

!syntax inputs /Physics/NavierStokes/WCNSFVScalarAdvection/WCNSFVScalarAdvectionPhysics

!syntax children /Physics/NavierStokes/WCNSFVScalarAdvection/WCNSFVScalarAdvectionPhysics
