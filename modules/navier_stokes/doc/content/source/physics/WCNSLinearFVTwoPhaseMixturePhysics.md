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
- [LinearFVScalarAdvection.md] for the scalar advection term, advected at the dispersed phase
  velocity $\bm{u}_d = \bm{u}_m + \bm{u}_{Md}$, where $\bm{u}_{Md} = (1 - c_d)
  \bm{u}_{slip,d}$ is the diffusion velocity and $c_d$ the dispersed phase mass fraction. The
  diffusion velocity is supplied by [LinearWCNSFV2PSlipVelocityFunctorMaterial.md], which derives
  it from the slip velocity it computes
- [LinearFVDiffusion.md] for the scalar diffusion term

Additionally, if a phase exchange coefficient is specified with the
[!param](/Physics/NavierStokes/TwoPhaseMixtureSegregated/WCNSLinearFVTwoPhaseMixturePhysics/alpha_exchange)
parameter, the phase exchange term is added using a [LinearFVReaction.md] and a [LinearFVSource.md],
which together reproduce the term implemented by [NSFVMixturePhaseInterface.md] in the nonlinear
finite volume discretization.

The momentum equations, if defined using a [WCNSLinearFVFlowPhysics.md], are modified in the presence of a two-phase
mixture. Density and viscosity should be set to their mixture values, see [#materials] for more information.
If specified with the
[!param](/Physics/NavierStokes/TwoPhaseMixtureSegregated/WCNSLinearFVTwoPhaseMixturePhysics/add_drift_flux_momentum_terms)
parameter, the diffusion (drift) stress is added to the momentum equations with the
[LinearWCNSFV2PMomentumDriftFlux.md] kernels, with its exact coefficient $\beta_d \beta_c /
\rho_m$ rather than the dilute limit $\alpha_d \rho_d$.

When an energy equation is present, the same parameter also adds the enthalpy carried by the
relative motion of the phases, with the [LinearWCNSFV2PEnergyDriftFlux.md] kernel. That term is
the energy counterpart of the diffusion stress and is proportional to the difference of the two
specific heats, so it vanishes when they are equal.

The fluid energy equation, if defined using a [WCNSLinearFVFluidHeatTransferPhysics.md], is modified in
the presence of a two-phase mixture. If specified with the
[!param](/Physics/NavierStokes/TwoPhaseMixtureSegregated/WCNSLinearFVTwoPhaseMixturePhysics/add_phase_change_energy_term)
parameter, a phase change source term is added to the fluid phase energy equation. The nonlinear
discretization uses the [NSFVPhaseChangeSource.md] kernel for this; because that term is the product of
a temperature-dependent coefficient and the temperature time derivative, the linear finite volume
implementation instead assembles it from a [LinearFVTimeDerivative.md] whose `factor` is that
coefficient, computed by a [ParsedFunctorMaterial.md]. The coefficient is lagged to the previous fixed
point iteration rather than differentiated, so the mushy zone may require tighter fixed point solve
tolerances than the nonlinear implementation.

!alert note
The advection slip momentum term is currently not supported in the linear finite volume discretization.

!alert note
Interfacial area transport is currently not supported in the linear finite volume discretization.

!alert note
Mesh skewness combined with buoyancy is currently not supported, as the reconstruction of the buoyancy
forces has not been implemented yet.

## Advection of the phase fraction id=phase_advection

The phase fraction is advected at the dispersed phase velocity
$\vec{u}_m + \vec{u}_{Md}$, so the face flux of the phase equation carries a drift contribution
in addition to the mixture flux supplied by the Rhie-Chow object. The two are interpolated
separately, each with its own upwind direction, because they may oppose one another and the donor
cell of their sum is the wrong donor for whichever of them runs against it. Holding each part in
donor cell form is what keeps the phase equation matrix an M-matrix and the phase fraction bounded.

The mixture flux takes the scheme named by
[!param](/Physics/NavierStokes/TwoPhaseMixtureSegregated/WCNSLinearFVTwoPhaseMixturePhysics/phase_advection_interpolation),
and so does the drift unless
[!param](/Physics/NavierStokes/TwoPhaseMixtureSegregated/WCNSLinearFVTwoPhaseMixturePhysics/phase_drift_advection_interpolation)
names one for it alone. That is the prerequisite for treating the drift flux differently from the
mixture flux, for instance by limiting it on its own. See [LinearFVScalarAdvection.md] for the
assembled form.

## Mixture fluid properties id=materials

The fluid properties of mixture fluids depend on the phase fraction of each phase.
The density, the dynamic viscosity and the thermal conductivity are volume-weighted averages of the
two phase values and are computed with a [NSFVMixtureFunctorMaterial.md].

The specific heat is not. For $\rho_m c_{p,m} T$ to be the mixture enthalpy density $\sum_k
\alpha_k \rho_k c_{p,k} T$, the mixture specific heat has to be the *mass*-weighted average

!equation
c_{p,m} = \frac{\alpha_d \rho_d c_{p,d} + (1 - \alpha_d) \rho_c c_{p,c}}{\rho_m}

so `cp_mixture` is created separately with a [ParsedFunctorMaterial.md] using this weighting. The
two weightings agree only when the two phase densities are equal.

These materials are defined by default by the `WCNSLinearFVTwoPhaseMixturePhysics` unless the
[!param](/Physics/NavierStokes/TwoPhaseMixtureSegregated/WCNSLinearFVTwoPhaseMixturePhysics/use_external_mixture_properties)
is set to true.

The gas mixture models defined in the fluid properties module cannot currently be used by this physics
without additional development.

!syntax parameters /Physics/NavierStokes/TwoPhaseMixtureSegregated/WCNSLinearFVTwoPhaseMixturePhysics

!syntax inputs /Physics/NavierStokes/TwoPhaseMixtureSegregated/WCNSLinearFVTwoPhaseMixturePhysics

!syntax children /Physics/NavierStokes/TwoPhaseMixtureSegregated/WCNSLinearFVTwoPhaseMixturePhysics
