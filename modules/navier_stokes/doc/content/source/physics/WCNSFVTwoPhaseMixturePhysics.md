# Navier Stokes Two Phase Mixtures / WCNSFVTwoPhaseMixturePhysics

!syntax description /Physics/NavierStokes/ScalarTransport/WCNSFVTwoPhaseMixturePhysics

## Equation(s)

This [Physics.md] adds terms to the flow and energy equations to account for the presence of a
two-phase mixture. If specified with the [!param](/Physics/NavierStokes/ScalarTransport/WCNSFVTwoPhaseMixturePhysics/add_phase_transport_equation)
parameter, it can also solve for the advection-diffusion equation of a moving phase fraction.

!alert note
If the other phase is solid, the [!param](/Physics/NavierStokes/ScalarTransport/WCNSFVTwoPhaseMixturePhysics/add_phase_transport_equation)
parameter should be set to false.

The phase advection-diffusion equation is:

!equation
\dfrac{\partial \phi}{\partial t} + \nabla \cdot (\phi \mathbf{v}) - \nabla \cdot (k \nabla \phi) -\alpha \phi = 0

where:

- $\phi$ is the phase fraction
- \mathbf{v} is the advecting velocity
- $k$ the phase diffusivity
- $\alpha$ is the phase exchange coefficient

The kernels created are:

- [FVFunctorTimeKernel.md] for the time derivative for a transient solve
- [INSFVScalarFieldAdvection.md] for the scalar advection term
- [FVDiffusion.md] for the scalar diffusion term

The momentum equations, if defined using a [WCNSFVFlowPhysics.md], are modified in the presence of a two-phase
mixture. Density and viscosity should be set to their mixture values, see [#materials] for more information.

If specified with the [!param](/Physics/NavierStokes/ScalarTransport/WCNSFVTwoPhaseMixturePhysics/add_drift_flux_momentum_terms) parameter,
a drift flux term is added to the momentum equations with the [WCNSFV2PMomentumDriftFlux.md] kernels.

If specified with the [!param](/Physics/NavierStokes/ScalarTransport/WCNSFVTwoPhaseMixturePhysics/add_advection_slip_term) parameter,
an advection slip term is added to the momentum equations with the [WCNSFV2PMomentumAdvectionSlip.md] kernels.

If specified with the [!param](/Physics/NavierStokes/ScalarTransport/WCNSFVTwoPhaseMixturePhysics/add_phase_change_term) parameter,
a phase change source term is added to the fluid phase energy equation with the [NSFVPhaseChangeSource.md] kernel.

## Mixture fluid properties id=materials

The fluid properties of mixture fluids depend on the phase fraction of each phase.
For two-phases, the properties can currently be computed with a [NSFVMixtureFunctorMaterial.md].
This material is defined by default by the `WCNSFVTwoPhaseMixturePhysics` unless the
[!param](/Physics/NavierStokes/ScalarTransport/WCNSFVTwoPhaseMixturePhysics/use_external_enthalpy_material)
is set to true.

The gas mixture models defined in the fluid properties module cannot currently be used with
[WCNSFVTwoPhaseMixturePhysics.md] without additional development.

!syntax parameters /Physics/NavierStokes/ScalarTransport/WCNSFVTwoPhaseMixturePhysics

!syntax inputs /Physics/NavierStokes/ScalarTransport/WCNSFVTwoPhaseMixturePhysics

!syntax children /Physics/NavierStokes/ScalarTransport/WCNSFVTwoPhaseMixturePhysics
