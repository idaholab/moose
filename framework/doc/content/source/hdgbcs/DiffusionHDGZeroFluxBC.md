# DiffusionHDGZeroFluxBC

This class implements the natural condition for the diffusion equation, e.g.

\begin{equation}
\vec{n} \cdot D \nabla u = 0
\end{equation}

in a hybridized discontinuous Galerkin (HDG) discretization. It should be paired
with a [DiffusionHDGKernel.md].

!syntax parameters /HDGBCs/DiffusionHDGZeroFluxBC

!syntax inputs /HDGBCs/DiffusionHDGZeroFluxBC

!syntax children /HDGBCs/DiffusionHDGZeroFluxBC
