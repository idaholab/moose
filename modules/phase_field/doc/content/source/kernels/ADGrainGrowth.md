# ADGrainGrowth

!syntax description /Kernels/ADGrainGrowth

ADGrainGrowth is the automatic differentiation version of the
[ACGrGrPoly](/ACGrGrPoly.md) Kernel and can be activated in the
[GrainGrowthAction](/GrainGrowthAction.md) through the
`use_automatic_differentiation` parameter.

The Kernel implements the bulk part of the grain growth model
[cite!moelans_quantitative_2008] Allen-Cahn equation
\begin{equation}
\left(L\mu(\eta^3-\eta+2\gamma\eta\sum_j\eta_j),\psi\right),
\end{equation}
where $L$ the mobility, $\eta$ the kernel variable, and $\eta_j$ are the other
order parameters. $\mu$ and $\gamma$ are model parameters contributing to the
grain boundary energy $\sigma$ as
\begin{equation}
\sigma = g(\gamma)\sqrt{\kappa\mu},
\end{equation}
where $\kappa$ is the gradient energy coefficient from
[ADACInterface](/ADACInterface.md) and $g$ is a function that needs to be
numerically determined (see [cite!moelans_quantitative_2008] for details).

$\kappa_{i,j}$ and $\gamma_{i,j} could be defined for specific grain boundaries
(grain pairs). This is provided by the [GBAnisotropy](/GBAnisotropy.md)
material.

!syntax parameters /Kernels/ADGrainGrowth

!syntax inputs /Kernels/ADGrainGrowth

!syntax children /Kernels/ADGrainGrowth

!bibtex bibliography
