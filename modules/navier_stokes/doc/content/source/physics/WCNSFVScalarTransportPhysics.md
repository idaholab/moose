# Navier Stokes Scalar Transport / WCNSFVScalarTransportPhysics

!syntax description /Physics/NavierStokes/ScalarTransport/WCNSFVScalarTransportPhysics

## Equation

This [Physics](Physics/index.md) object creates the kernels and boundary conditions to solve the advection-diffusion-reaction
equation for several scalar quantities advected by the flow.

!equation
\dfrac{\partial \phi_i}{\partial t} + \nabla \cdot (\phi_i \mathbf{v}) - \nabla \cdot (k_i \nabla \phi_i) - Q_i - \lambda_i \phi_i = 0

where:

- $\phi_i$ is the i-th scalar quantity
- \mathbf{v} is the advecting velocity
- $k_i$ the i-th scalar diffusivity
- $Q_i$ is the i-th precursor source
- $\lambda_i$ is a reaction coefficient. It should be negative for a loss term

The kernels created are:

- [FVFunctorTimeKernel.md] for the time derivative for a transient solve
- [INSFVScalarFieldAdvection.md] for the scalar advection term
- [FVDiffusion.md] for the scalar diffusion term
- [FVCoupledForce.md] for the reaction terms
- [FVCoupledForce.md] for the source terms

## Coupling with other Physics

Scalar advection equations can be solved concurrently with the flow equations by combining the [WCNSFVFlowPhysics.md] with the `WCNSFVScalarTransportPhysics`.
The following input performs this coupling for incompressible flow in a 2D channel.
No system parameters are passed, so the equations are solved in a fully coupled manner in the same [nonlinear system](systems/NonlinearSystem.md).

!listing test/tests/finite_volume/ins/channel-flow/2d-scalar-transport-physics.i block=Physics

!syntax parameters /Physics/NavierStokes/ScalarTransport/WCNSFVScalarTransportPhysics

!syntax inputs /Physics/NavierStokes/ScalarTransport/WCNSFVScalarTransportPhysics

!syntax children /Physics/NavierStokes/ScalarTransport/WCNSFVScalarTransportPhysics
