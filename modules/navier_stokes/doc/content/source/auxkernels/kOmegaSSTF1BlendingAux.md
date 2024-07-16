# kOmegaSSTF1BlendingAux

Computes the $F_1$ blending function for the $k-\omega$ SST model.
The function is defined as follows:

\begin{equation}
F_1 = \operatorname{tanh} \left[ \left[ \operatorname{min} \left( \operatorname{max} \left( \frac{\sqrt{k}}{\beta^* \omega d}, \frac{500 \mu}{\rho d^2 \omega} \right), \frac{2k}{d^2 {CD}_{k \omega}} \right) \right]^4 \right] \,,
\end{equation}

where:

- $k$ is the turbulent kinetic energy,
- $\omega$ is the turbulent kinetic energy specific dissipation rate,
- d$ is the distance to the nearest wall provided in the [!param](/AuxKernels/kOmegaSSTF1BlendingAux/wall_distance) functor,
- $\mu$ is the dynamic viscosity,
- $\rho$ is the density,
- ${CD}_{k \omega} = \operatorname{max} \left( \frac{\nabla k \cdot \nabla \omega}{\omega}, 10^{-20} \right)$ is the positive part of the cross diffusion term,
- $\beta^* = 0.09$ is a closure parameter.

To calculate the distance to the nearest wall, we recommend using [WallDistanceAux](WallDistanceAux.md).

In practice, the $F_1$ function provides the blend between the $k-\epsilon$ and the $k-\omega$ models.
The model behaves like a $k-\omega$ when $F_1 \rightarrow 1$ and like a $k-\epsilon$ when $F_1 \rightarrow 0$.
Hence, one should expect $F_1 \rightarrow 1$ near the walls, while $F_1 \rightarrow 0$ for the bulk of the flow.

!syntax parameters /AuxKernels/kOmegaSSTF1BlendingAux

!syntax inputs /AuxKernels/kOmegaSSTF1BlendingAux

!syntax children /AuxKernels/kOmegaSSTF1BlendingAux
