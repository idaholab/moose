# WCNSFV2PMomentumDriftFlux

This object implements the contribution to the momentum equation
from the drift flux term

\begin{equation}
- \nabla \cdot \alpha_d \rho_d \bm{v}_{slip,d} \bm{v}_{slip,d} \,,
\end{equation}

where:

- $\alpha_d$ is the fraction of the dispersed phase $d$,
- $\rho_d$ is the density of the dispersed phase $d$,
- $\bm{v}_{slip,d}$ is the slip velocity of the dispersed phase $d$.

The user can set the slip velocity from their specific calculations.
However, we recommend the usage of [WCNSFV2PSlipVelocityFunctorMaterial.md] for
computing the slip velocity.

This term can be interpreted as the extra convection that is added
due to the particles being transported in the flow field.

!alert note
If the mixture model is used to capture more than one dispersed phase,
a different `WCNSFV2PMomentumDriftFlux` kernel should be added for each
of the transported phases with the corresponding slip velocity for
each phase.

!alert note
Because the mixture density is computed inside `WCNSFV2PMomentumDriftFlux`, this kernel
is not compatible with mixture fluid properties in the fluid properties module that do
not use a linear volume mixing for densities.

!syntax parameters /FVKernels/WCNSFV2PMomentumDriftFlux

!syntax inputs /FVKernels/WCNSFV2PMomentumDriftFlux

!syntax children /FVKernels/WCNSFV2PMomentumDriftFlux
