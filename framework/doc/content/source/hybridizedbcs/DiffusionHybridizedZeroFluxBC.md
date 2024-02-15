# DiffusionHybridizedZeroFluxBC

This class implements the natural condition for the diffusion equation, e.g.

\begin{equation}
\vec{n} \cdot D \nabla u = 0
\end{equation}

in a hybridized discontinuous Galerkin (HDG) discretization. It should be paired
with a [DiffusionHybridizedKernel.md].

!syntax parameters /HybridizedBCs/DiffusionHybridizedZeroFluxBC

!syntax inputs /HybridizedBCs/DiffusionHybridizedZeroFluxBC

!syntax children /HybridizedBCs/DiffusionHybridizedZeroFluxBC
