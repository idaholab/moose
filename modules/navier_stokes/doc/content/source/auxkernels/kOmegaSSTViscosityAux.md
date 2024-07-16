# kOmegaSSTViscosityAux

Computes the turbulent viscosity for the $k-\omega$-SST model.

This is the auxiliary kernel used to calculate the dynamic turbulent viscosity as follows:

\begin{equation}
\mu_t = \rho k \operatorname{min} \left( \frac{\alpha^*}{\omega}, \frac{a_1}{S F_2} \right)\,,
\end{equation}

where:

- $\rho$ is the density,
- k$ is the turbulent kinetic energy,
- $\omega$ is the turbulent kinetic energy specific dissipation rate,
- a_1 = 0.31$ is a closure parameter for the turbulent viscosity boundary,
- S = || 0.5 (\nabla \vec{u} + \nabla \vec{u}^T) ||$ is the shear strain rate norm, where $\vec{u}$ is the velocity vector,
- $F_2$ is a blending function, which should be calculated with [kOmegaSSTF2BlendingAux](kOmegaSSTF2BlendingAux.md),
- $\alpha^*$ is a closure parameter that depends on the corrections implemented for the model.

The $\alpha^*$ closure parameter is defined as follows

#### Standard $k-\omega$-SST Model

\begin{equation}
\alpha^* = 1 \,,
\end{equation}

#### Low Reynolds $k-\omega$-SST Model

\begin{equation}
\alpha^* = F_1 \frac{\beta/3 + Re_{\tau}/Re_k}{1.0 + Re_{\tau}/Re_k} + (1 - F_1) \alpha^*_2\,,
\end{equation}

where:

- $F_1$ is a blending function that should be calculated with [kOmegaSSTF1BlendingAux](kOmegaSSTF1BlendingAux.md),
- $\beta = F_1 \beta_1 + (1 - F_1) \beta_2$, with $\beta_1 = 0.075$ and $\beta_2 = 0.0828$, is a closing parameter,
- $Re_{\tau} = \frac{\rho k}{\mu \omega}$ is the non-equilibrium friction Reynolds number,
- $Re_{\omega} = 2.95$ is a closure parameter for the low Reynolds limit,
- $\alpha^*_2 = 1.0$ is a closure parameter for the bulk scale.


By setting parameter [!param](/AuxKernels/kOmegaSSTViscosityAux/bulk_wall_treatment) to `true`, the
kernel allows us to set the value of the cells on the boundaries specified in
[!param](/AuxKernels/kOmegaSSTViscosityAux/walls) to the dynamic turbulent viscosity predicted
from the law of the wall or non-equilibrium wall functions.
See [INSFVTurbulentViscosityWallFunction](INSFVTurbulentViscosityWallFunction.md) for more
details about the near-wall implementation.

!alert note
If the boundary conditions for the dynamic turbulent viscosity are already set via [INSFVTurbulentViscosityWallFunction](INSFVTurbulentViscosityWallFunction.md),
there is no need to add bulk wall treatment, i.e., we should set
[!param](/AuxKernels/kOmegaSSTViscosityAux/bulk_wall_treatment) to `false`.
This type of bulk wall treatment is mainly designed for porous media formulations
with large computational cells.

!syntax parameters /AuxKernels/kOmegaSSTViscosityAux

!syntax inputs /AuxKernels/kOmegaSSTViscosityAux

!syntax children /AuxKernels/kOmegaSSTViscosityAux
