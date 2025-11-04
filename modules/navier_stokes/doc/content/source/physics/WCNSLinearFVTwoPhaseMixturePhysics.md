# Navier Stokes Two Phase Mixture using a Linear Finite Volume discretization / WCNSLinearFVTwoPhaseMixturePhysics

!syntax description /Physics/NavierStokes/TwoPhaseMixtureSegregated/WCNSLinearFVTwoPhaseMixturePhysics

## Equation(s)

This [Physics](Physics/index.md) adds terms to the flow and energy equations to account for the presence of a
two-phase mixture. If specified with the [!param](/Physics/NavierStokes/TwoPhaseMixtureSegregated/WCNSLinearFVTwoPhaseMixturePhysics/add_phase_transport_equation)
parameter, it can also solve for the advection-diffusion equation of a moving phase fraction.

!alert note
If the other phase is solid, the [!param](/Physics/NavierStokes/TwoPhaseMixtureSegregated/WCNSLinearFVTwoPhaseMixturePhysics/add_phase_transport_equation)
parameter should be set to false.

The phase advection-diffusion equation is:

!equation
\dfrac{\partial \phi}{\partial t} + \nabla \cdot (\phi \mathbf{v}) - \nabla \cdot (k \nabla \phi) -\alpha \phi = 0

where:

- $\phi$ is the phase fraction
- $\mathbf{v}$ is the advecting velocity
- $k$ the phase diffusivity
- $\alpha$ is the phase exchange coefficient

The kernels created are:

- [LinearFVTimeDerivative.md] for the time derivative for a transient solve
- [LinearFVScalarAdvection.md] for the scalar advection term
- [LinearFVDiffusion.md] for the scalar diffusion term

The momentum equations, if defined using a [WCNSLinearFVFlowPhysics.md], are modified in the presence of a two-phase
mixture. Density and viscosity should be set to their mixture values, see [#materials] for more information.

!alert note
Drift flux and advection slip terms are currently not supported in the linear finite volume discretization.

!alert note
Phase interface terms are currently not supported in the linear finite volume discretization.

!alert note
Phase exchange is not currently supported in the linear finite volume discretization.

## Mixture fluid properties id=materials

The fluid properties of mixture fluids depend on the phase fraction of each phase.
For two phases, the properties can currently be computed with a [NSFVMixtureFunctorMaterial.md].
This material is defined by default by the `WCNSLinearFVTwoPhaseMixturePhysics` unless the
[!param](/Physics/NavierStokes/TwoPhaseMixtureSegregated/WCNSLinearFVTwoPhaseMixturePhysics/use_external_mixture_properties)
is set to true.

The gas mixture models defined in the fluid properties module cannot currently be used by this physics
without additional development.

!syntax parameters /Physics/NavierStokes/TwoPhaseMixtureSegregated/WCNSLinearFVTwoPhaseMixturePhysics

!syntax inputs /Physics/NavierStokes/TwoPhaseMixtureSegregated/WCNSLinearFVTwoPhaseMixturePhysics

!syntax children /Physics/NavierStokes/TwoPhaseMixtureSegregated/WCNSLinearFVTwoPhaseMixturePhysics
