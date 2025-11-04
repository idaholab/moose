# kEpsilonViscosityAux

This is the auxiliary kernel used to compute the dynamic turbulent viscosity

\begin{equation}
\mu_t = \rho C_{\mu} k T_e \,,
\end{equation}

where:

- $\rho$ is the density,
- $C_{\mu} = 0.09$ is a closure parameter,
- $k$ is the turbulent kinetic energy,
- $\epsilon$ is the turbulent kinetic energy dissipation rate.
- $T_e = max( \frac{k}{\epsilon} , \sqrt(\frac{\nu}{\epsilon}) )$.

By setting parameter [!param](/AuxKernels/kEpsilonViscosityAux/bulk_wall_treatment) to `true`, the
kernel allows us to set the value of the cells on the boundaries specified in
[!param](/AuxKernels/kEpsilonViscosityAux/walls) to the dynamic turbulent viscosity predicted
from the law of the wall or non-equilibrium wall functions.
See [INSFVTurbulentViscosityWallFunction](INSFVTurbulentViscosityWallFunction.md) for more
details about the near-wall implementation.

!alert note
If the boundary conditions for the dynamic turbulent viscosity are already set via [INSFVTurbulentViscosityWallFunction](INSFVTurbulentViscosityWallFunction.md),
there is no need to add bulk wall treatment, i.e., we should set
[!param](/AuxKernels/kEpsilonViscosityAux/bulk_wall_treatment) to `false`.
This type of bulk wall treatment is mainly designed for porous media formulations
with large computational cells.

!syntax parameters /AuxKernels/kEpsilonViscosityAux

!syntax inputs /AuxKernels/kEpsilonViscosityAux

!syntax children /AuxKernels/kEpsilonViscosityAux
