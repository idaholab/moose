# LinearWCNSFV2PMomentumDriftFlux

This object implements the diffusion, or drift, stress of the two-phase mixture model in the
linear finite volume discretization. It is the only term the mixture momentum equation carries
beyond its single-phase counterpart.

## Formulation

Summing the phase momentum equations and substituting $\bm{u}_k = \bm{u}_m + \bm{u}_{Mk}$, where
$\bm{u}_{Mk} = \bm{u}_k - \bm{u}_m$ is the diffusion velocity of phase $k$, gives

\begin{equation}
\sum_k \nabla \cdot \left( \alpha_k \rho_k \bm{u}_k \bm{u}_k \right)
  = \nabla \cdot \left( \rho_m \bm{u}_m \bm{u}_m \right)
  + \nabla \cdot \left( \sum_k \alpha_k \rho_k \bm{u}_{Mk} \bm{u}_{Mk} \right) \,,
\end{equation}

the cross terms vanishing identically because $\sum_k \alpha_k \rho_k \bm{u}_{Mk} = 0$ is what
defines the mass-averaged mixture velocity. The first term is the ordinary convection assembled by
[LinearWCNSFVMomentumFlux.md]. The second is this kernel, assembled on the left hand side,

\begin{equation}
+ \nabla \cdot \left( \frac{\beta_d \beta_c}{\rho_m}\, \bm{u}_{slip,d} \otimes \bm{u}_{slip,d}
\right) \,,
\end{equation}

where:

- $\beta_d = \alpha_d \rho_d$ and $\beta_c = (1 - \alpha_d) \rho_c$ are the partial densities of
  the dispersed and continuous phases,
- $\rho_m = \beta_d + \beta_c$ is the mixture density,
- $\bm{u}_{slip,d} = \bm{u}_d - \bm{u}_c$ is the slip velocity of the dispersed phase.

The coefficient $\beta_d \beta_c / \rho_m$ is exact. It follows from
$\tau_{Dm} = -\rho_m c_d (1 - c_d) \bm{u}_{slip} \bm{u}_{slip}$ with $c_d = \alpha_d \rho_d /
\rho_m$ the dispersed phase mass fraction, equation (76) of [!cite](manninen1996mixture). The
dilute limit $\beta_c / \rho_m \to 1$ reduces it to $\alpha_d \rho_d$, which is the form quoted in
much of the reactor literature; it is accurate only when the mass fraction of the dispersed phase
is small, and it is not what this kernel implements.

The term can be interpreted as the momentum transported by the relative motion of the phases.
Its energy counterpart is [LinearWCNSFV2PEnergyDriftFlux.md].

## Sign convention

The term is positive on the left hand side, equivalently $+\nabla \cdot \tau_{Dm}$ on the right
hand side with $\tau_{Dm} = -\sum_k \alpha_k \rho_k \bm{u}_{Mk} \bm{u}_{Mk}$. This matches
equations (18) and (21) of [!cite](manninen1996mixture) and the `driftFluxFoam` solver of OpenFOAM,
which adds `fvc::div(tauDm)` to the left hand side of its momentum equation with
`tauDm = betad*sqr(Udm) + betac*sqr(Ucm)`.

!alert warning
The nonlinear counterpart of this object, [WCNSFV2PMomentumDriftFlux.md], carries the opposite
sign and the dilute coefficient. The two discretizations therefore do not solve the same momentum
equation, and results obtained with one are not comparable with the other for this term.

## Numerical treatment

The term has no genuine linear dependence on the velocity component being solved for: the slip
velocity comes from an algebraic closure driven by gravity and by the pressure gradient, not by the
local cell value. The matrix contribution is therefore a deferred correction rather than a
linearization, with a coefficient bounded by the scale of the term itself and given an upwind
structure so that both diagonals stay non-negative. The right hand side carries the exact flux
minus the surrogate, so the two cancel at convergence and the converged solution does not depend on
the surrogate.

The user can set the slip velocity from their own calculations. However, we recommend the usage of
[LinearWCNSFV2PSlipVelocityFunctorMaterial.md] for computing it, which also supplies the diffusion
velocity that the phase transport equation requires.

!alert note
If the mixture model is used to capture more than one dispersed phase, a different
`LinearWCNSFV2PMomentumDriftFlux` kernel should be added for each of the transported phases with
the corresponding slip velocity for each phase.

!alert note
Because the mixture density is computed inside `LinearWCNSFV2PMomentumDriftFlux` from the two phase
densities and the phase fraction, this kernel is not compatible with mixture fluid properties in
the fluid properties module that do not use a linear volume mixing for densities.

!syntax parameters /LinearFVKernels/LinearWCNSFV2PMomentumDriftFlux

!syntax inputs /LinearFVKernels/LinearWCNSFV2PMomentumDriftFlux

!syntax children /LinearFVKernels/LinearWCNSFV2PMomentumDriftFlux
