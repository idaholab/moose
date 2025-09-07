# ADFluxDivergence

!syntax description /Kernels/ADFluxDivergence

## Description

The `ADFluxDivergence` kernel forms the weak divergence of a material-provided
flux vector $\mathbf{J}$. The residual contribution is

\begin{equation}
R_i = (\nabla \psi_i, \mathbf{J}),
\end{equation}

which corresponds to $-\nabla \cdot \mathbf{J}$ in the strong form. The flux is
retrieved from a material property named `flux` (or `base_name + flux`).

The example below employs `ADFluxFromGradientMaterial`, which computes a diffusive
flux $\mathbf{J} = -D\nabla u$ from the gradient of the coupled variable `u`.

## Example Input File

!listing test/tests/kernels/flux_divergence/ad_flux_divergence.i block=Materials/flux Kernels/flux

!syntax parameters /Kernels/ADFluxDivergence

!syntax inputs /Kernels/ADFluxDivergence

!syntax children /Kernels/ADFluxDivergence

