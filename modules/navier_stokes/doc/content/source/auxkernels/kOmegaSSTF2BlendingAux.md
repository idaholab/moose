# kOmegaSSTF2BlendingAux

Computes the $F_2$ blending function for the $k-\omega$ SST model.
The function is defined as follows:

\begin{equation}
F_2 = \operatorname{tanh} \left[ \left[ \operatorname{max} \left( \frac{2 \sqrt{k}}{\beta^* \omega d}, \frac{500 \mu}{\rho d^2 \omega} \right) \right]^2 \right] \,,
\end{equation}

where:

- $k$ is the turbulent kinetic energy,
- $\omega$ is the turbulent kinetic energy specific dissipation rate,
- d$ is the distance to the nearest wall provided in the [!param](/AuxKernels/kOmegaSSTF2BlendingAux/wall_distance) functor,
- $\mu$ is the dynamic viscosity,
- $\rho$ is the density,
- $\beta^* = 0.09$ is a closure parameter.

To calculate the distance to the nearest wall, we recommend using [WallDistanceAux](WallDistanceAux.md).

!syntax parameters /AuxKernels/kOmegaSSTF2BlendingAux

!syntax inputs /AuxKernels/kOmegaSSTF2BlendingAux

!syntax children /AuxKernels/kOmegaSSTF2BlendingAux
