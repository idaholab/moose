# Navier Stokes Linear Scalar Transport / WCNSLinearFVScalarTransportPhysics

!syntax description /Physics/NavierStokes/ScalarTransportSegregated/WCNSLinearFVScalarTransportPhysics

## Equation

This [Physics](Physics/index.md) object creates the kernels and boundary conditions to solve the advection-diffusion-reaction
equation for several scalar quantities advected by the flow.

!equation
\dfrac{\partial \phi_i}{\partial t} + \nabla \cdot (\phi_i \mathbf{v}) - \nabla \cdot (k_i \nabla \phi_i) - Q_i - \lambda_i \phi_i = 0

where:

- $\phi_i$ is the i-th scalar quantity
- \mathbf{v} is the advecting velocity
- $k_i$ the i-th scalar diffusivity
- $Q_i$ is the i-th scalar source
- $\lambda_i$ is a reaction coefficient. It should be negative for a loss term

The kernels created are:

- [LinearFVScalarAdvection.md] for the scalar advection term
- [LinearFVDiffusion.md] for the scalar diffusion term
- [LinearFVSource.md] for the source terms

Reaction terms can be expressed as source terms by using a negative coefficient in the
[!param](/Physics/NavierStokes/ScalarTransportSegregated/WCNSLinearFVScalarTransportPhysics/passive_scalar_coupled_source_coeff)
parameter, and the scalar variable as one of the
[!param](/Physics/NavierStokes/ScalarTransportSegregated/WCNSLinearFVScalarTransportPhysics/passive_scalar_coupled_source)(s).

## Coupling with other Physics

Scalar advection equations can be solved concurrently with the flow equations by combining the [WCNSLinearFVFlowPhysics.md] with the `WCNSLinearFVScalarTransportPhysics`.
The following input performs this coupling for incompressible flow in a 2D channel.

!listing test/tests/finite_volume/ins/channel-flow/linear-segregated/2d-scalar/channel-physics.i block=Physics

!syntax parameters /Physics/NavierStokes/ScalarTransportSegregated/WCNSLinearFVScalarTransportPhysics

!syntax inputs /Physics/NavierStokes/ScalarTransportSegregated/WCNSLinearFVScalarTransportPhysics

!syntax children /Physics/NavierStokes/ScalarTransportSegregated/WCNSLinearFVScalarTransportPhysics
