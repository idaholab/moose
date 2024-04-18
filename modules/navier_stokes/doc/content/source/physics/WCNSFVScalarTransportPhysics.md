# WCNSFVScalarTransportPhysics

!syntax description /Physics/NavierStokes/WCNSFVScalarTransport/WCNSFVScalarTransportPhysics

## Coupling with other Physics

Scalar avection equations can be solved concurrently with the flow equations by combining the [WCNSFVFlowPhysics.md] with the `WCNSFVScalarTransportPhysics`.
The following input performs this coupling for incompressible flow in a 2D channel.
No system parameters are passed, so the equations are solved in a fully coupled manner in the same [nonlinear system](systems/NonlinearSystem.md).

!listing test/tests/finite_volume/ins/channel-flow/2d-scalar-transport-physics.i block=Physics

!syntax parameters /Physics/NavierStokes/WCNSFVScalarTransport/WCNSFVScalarTransportPhysics

!syntax inputs /Physics/NavierStokes/WCNSFVScalarTransport/WCNSFVScalarTransportPhysics

!syntax children /Physics/NavierStokes/WCNSFVScalarTransport/WCNSFVScalarTransportPhysics
