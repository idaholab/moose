# MassFluxPenalty

This class introduces a jump correction for grad-div stabilization (see [GradDiv.md]) for discontinuous Galerkin methods. With its addition, we achive the neat equality that the summation of the matrices created through this kernel and `GradDiv` is equivalent to

\begin{equation}
B M_p^{-1} B^T
\end{equation}

where $B$ is the matrix corresponding to the $\nabla p$ term in the Navier-Stokes momentum equation, $M_p^{-1}$ is the inverse of the pressure mass matrix, and $B^T$ is the transpose of the gradient operator, e.g. divergence, corresponding to the conservation of mass equation. Auxiliary matrices created from `GradDiv` and this kernel can be used to build scalable field-split preconditioners as outlined in [!citep](benzi2006augmented) and [!citep](farrell2019augmented).

!syntax parameters /DGKernels/MassFluxPenalty

!syntax inputs /DGKernels/MassFluxPenalty

!syntax children /DGKernels/MassFluxPenalty
