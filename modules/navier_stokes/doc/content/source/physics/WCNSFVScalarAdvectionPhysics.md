# WCNSFVScalarAdvectionPhysics

!syntax description /Physics/WCNSFVScalarAdvection

See the main [WCNSFVPhysics.md] page for an overview of the syntax and capabilities of the
weakly-compressible Navier Stokes `Physics` syntax.

## Coupling with other Physics

Scalar avection equations can be solved concurrently with the flow equations by combining the [WCNSFVFlowPhysics.md] with the `WCNSFVScalarAdvectionPhysics`.
The following input performs this coupling for incompressible flow in a 2D channel.
No system parameters are passed, so the equations are solved in a fully coupled manner in the same [nonlinear system](systems/NonlinearSystem.md).

!listing test/tests/finite_volume/ins/channel-flow/2d-scalar-transport-physics.i block=Physics

!syntax parameters /Physics/WCNSFVScalarAdvection

!syntax inputs /Physics/WCNSFVScalarAdvection

!syntax children /Physics/WCNSFVScalarAdvection
