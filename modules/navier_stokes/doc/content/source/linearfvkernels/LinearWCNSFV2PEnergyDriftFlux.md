# LinearWCNSFV2PEnergyDriftFlux

This object implements the enthalpy carried by the relative motion of the phases of a two-phase
mixture, in the linear finite volume discretization. It is the energy counterpart of the diffusion
stress assembled by [LinearWCNSFV2PMomentumDriftFlux.md], and it is the term written
$q_p = \sum_k \alpha_k \rho_k e_k \bm{u}_k$ in the multiphase literature.

## Formulation

The exact advective term of the mixture energy equation is the sum over the phases of each phase
carrying its own enthalpy at its own velocity, $\nabla \cdot \sum_k \alpha_k \rho_k h_k \bm{u}_k$.
Substituting $\bm{u}_k = \bm{u}_m + \bm{u}_{Mk}$ splits it into the mixture term that
[LinearFVEnergyAdvection.md] already assembles plus a remainder,

\begin{equation}
\nabla \cdot \sum_k \alpha_k \rho_k h_k \bm{u}_k
  = \nabla \cdot \left( \rho_m h_m \bm{u}_m \right)
  + \nabla \cdot \left( \sum_k \alpha_k \rho_k h_k \bm{u}_{Mk} \right) \,.
\end{equation}

For a single dispersed phase, using $\beta_c \bm{u}_{Mc} = -\beta_d \bm{u}_{Md}$ and
$\bm{u}_{Md} = (\beta_c / \rho_m) \bm{u}_{slip,d}$, the remainder collapses onto the slip velocity
with the same coefficient that carries the diffusion stress,

\begin{equation}
\sum_k \alpha_k \rho_k h_k \bm{u}_{Mk}
  = \frac{\beta_d \beta_c}{\rho_m} \left( h_d - h_c \right) \bm{u}_{slip,d} \,.
\end{equation}

The mixture model assumes thermal equilibrium between the phases, so both are at the mixture
temperature and $h_k = c_{p,k} T$. This kernel therefore assembles, on the left hand side,

\begin{equation}
+ \nabla \cdot \left[ \frac{\beta_d \beta_c}{\rho_m}
  \left( c_{p,d} - c_{p,c} \right) T\, \bm{u}_{slip,d} \right] \,,
\end{equation}

where $\beta_d = \alpha_d \rho_d$, $\beta_c = (1 - \alpha_d) \rho_c$ and $\rho_m = \beta_d +
\beta_c$. The term is proportional to the difference of the two specific heats and vanishes
identically when they are equal. It is linear in the advected temperature and is assembled
implicitly, with the interpolation selected by `advected_interp_method`.

The phase-summed advective term is $\nabla \cdot \sum_k \alpha_k \bm{u}_k (\rho_k E_k + p)$; see
[!cite](manninen1996mixture) equation (28) for the slip to diffusion velocity conversion.

## Mixture specific heat

For $\rho_m h_m$ to be the mixture enthalpy density $\sum_k \alpha_k \rho_k c_{p,k} T$, the mixture
specific heat multiplying $\rho_m$ must be the mass-weighted average

\begin{equation}
c_{p,m} = \frac{\beta_d c_{p,d} + \beta_c c_{p,c}}{\rho_m} \,,
\end{equation}

not the volume-weighted average used for the density, the viscosity and the thermal conductivity.
[WCNSLinearFVTwoPhaseMixturePhysics.md] creates `cp_mixture` with this weighting.

## Permeable boundaries

The dispersed phase cannot cross an impermeable wall, so no enthalpy may be carried through one by
the relative motion of the phases. The flux is applied on every internal face and on the boundaries
listed in `slip_boundaries`, normally the inlets and the outlets, and nowhere else. Carrying a
temperature boundary condition is not evidence that a boundary is permeable: a wall may legitimately
pin the temperature. [WCNSLinearFVTwoPhaseMixturePhysics.md] fills `slip_boundaries` from the flow
inlets and outlets automatically.

Because the term advects the temperature, it needs a boundary value to act on, and it contributes
only on boundaries where the temperature carries a boundary condition. `force_boundary_execution`
is deliberately left at its default of `false`.

!alert note
This term has no counterpart in the nonlinear finite volume discretization of the mixture model,
which advects the mixture enthalpy at the mixture velocity only.

!syntax parameters /LinearFVKernels/LinearWCNSFV2PEnergyDriftFlux

!syntax inputs /LinearFVKernels/LinearWCNSFV2PEnergyDriftFlux

!syntax children /LinearFVKernels/LinearWCNSFV2PEnergyDriftFlux
