# WCNSFV2PSlipVelocityFunctorMaterial

This material computes the slip velocity between a dispersed phase and the continuous phase for
the two-phase mixture model, and optionally the drift velocity derived from it. It is the
automatically differentiated counterpart of
[LinearWCNSFV2PSlipVelocityFunctorMaterial.md]; the two are one templated implementation and
evaluate the same closure, so see that page for the drag laws and the iteration that solves them.

## The slip velocity

The slip velocity is modeled as follows:

\begin{equation}
  \bm{u}_{slip,d} = \frac{\tau_d}{f_{drag}} \frac{\rho_d - \rho_m}{\rho_d} \bm{a} \,,
\end{equation}

where:

- $\tau_d$ is the particle relaxation time,
- $f_{drag}$ is the drag function,
- $\rho_d$ is the density of the dispersed phase,
- $\rho_m$ is the density of the mixture,
- $\bm{a}$ is the particle acceleration vector.

This is equation (58) of [!cite](manninen1996mixture). The density subtracted from $\rho_d$ in
the buoyancy factor is the *mixture* density, so the `rho` parameter should be given
`rho_mixture` and not the continuous phase density; the two agree only in the dilute limit.

The particle relaxation time is modeled as follows [!cite](bilicki1990dragmodel):

\begin{equation}
  \tau_d = \frac{\rho_d d_d^2}{18 \mu_c} \,,
\end{equation}

where:

- $d_d$ is the particle diameter,
- $\mu_c$ is the dynamic viscosity of the continuous phase, supplied under `mu`.

The acceleration vector is the particle acceleration vector:

\begin{equation}
  \bm{a} = \bm{g} + \frac{\bm{f}}{\rho_m} - \bm{u}_m \cdot \nabla \bm{u}_m - \frac{\partial \bm{u}_m}{dt} \,,
\end{equation}

where:

- $\bm{g}$ is the acceleration of gravity,
- $\bm{f}$ is the volumetric force,
- $\bm{u}_m$ is the mixture velocity.

## The drift velocity

The slip velocity above is measured against the continuous phase,
$\bm{u}_{slip,d} = \bm{u}_d - \bm{u}_c$. The conservation equations of the mixture model are
written against the centre of mass of the mixture, and the velocity of the dispersed phase
relative to that is the diffusion, or drift, velocity $\bm{u}_{Md} = \bm{u}_d - \bm{u}_m$. The
two are related by the dispersed phase mass fraction $c_d = \alpha_d \rho_d / \rho_m$,

\begin{equation}
\bm{u}_{Md} = \left( 1 - c_d \right) \bm{u}_{slip,d} \,,
\end{equation}

equation (28) of [!cite](manninen1996mixture). It is declared under `drift_velocity_name`, which
needs `fraction_dispersed` for the conversion, and it is the velocity the phase transport
equation must be advected at: the dispersed phase moves at $\bm{u}_m + \bm{u}_{Md}$, so
[INSFVScalarFieldAdvection.md] takes the drift velocity and not the slip velocity. Advecting the
phase at $\bm{u}_m + \bm{u}_{slip,d}$ instead is the dilute approximation $c_d \to 0$.

The slip velocity itself is what [WCNSFV2PMomentumDriftFlux.md] consumes, the diffusion stress
being a sum over both phases that collapses onto the slip velocity for a single dispersed phase.

`volumetric_drift_velocity_name` declares $(\alpha_d - c_d) \bm{u}_{slip,d}$, the difference
between the volume averaged and the mass averaged mixture velocity, which the pressure work of
the energy equation carries.

## The drag function

With `use_dispersed_phase_drag_model = true` the drag is solved together with the force balance,
so that the particle Reynolds number

!equation
Re_p = \frac{\rho_c d_d |\bm{u}_{slip,d}|}{\mu_c}

is formed from the slip velocity and the continuous phase properties, which is its definition,
equation (39) of [!cite](manninen1996mixture). The continuous phase density it needs is supplied
under `rho_c`. The alternative is to impose the drag directly through `linear_coef_name`; the two
are mutually exclusive.

[!param](/FunctorMaterials/WCNSFV2PSlipVelocityFunctorMaterial/drag_model) selects the law, and
`swarm_exponent` applies the hindrance factor $(1 - \alpha_d)^p$ that the two single particle
laws need in a swarm.

!syntax parameters /FunctorMaterials/WCNSFV2PSlipVelocityFunctorMaterial

!syntax inputs /FunctorMaterials/WCNSFV2PSlipVelocityFunctorMaterial

!syntax children /FunctorMaterials/WCNSFV2PSlipVelocityFunctorMaterial
