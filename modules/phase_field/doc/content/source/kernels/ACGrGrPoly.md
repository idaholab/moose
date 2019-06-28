# ACGrGrPoly

!syntax description /Kernels/ACGrGrPoly

This Kernel implements the bulk part of the grain growth model
[!cite](moelans_quantitative_2008) Allen-Cahn equation
\begin{equation}
\left(L\mu(\eta_i^3-\eta_i+2\gamma\eta_i\sum_{j\neq i}\eta_j^2),\psi\right),
\end{equation}
where $L$ is the mobility, $\eta_i$ the kernel variable, and $\eta_j$ are the
other order parameters. $\mu$ and $\gamma$ are model parameters contributing to
the grain boundary energy $\sigma$ as
\begin{equation}
\sigma = g(\gamma)\sqrt{\kappa\mu},
\end{equation}
where $\kappa$ is the gradient energy coefficient from
[ACInterface](/ACInterface.md) and $g$ is a function that needs to be
numerically determined (see [!cite](moelans_quantitative_2008) for details).

$\kappa_{i,j}$ and $\gamma_{i,j}$ could be defined for specific grain boundaries
(grain pairs). This is provided by the [GBAnisotropy](/GBAnisotropy.md)
material.

An automatic differentiation version of this kernel is implemented in
[ADGrainGrowth](/ADGrainGrowth.md).

!syntax parameters /Kernels/ACGrGrPoly

!syntax inputs /Kernels/ACGrGrPoly

!syntax children /Kernels/ACGrGrPoly

!bibtex bibliography
