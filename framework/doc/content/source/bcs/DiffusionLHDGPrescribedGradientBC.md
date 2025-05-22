# DiffusionLHDGPrescribedGradientBC

In a hybridized discontinuous Galerkin (HDG) discretization, this class
prescribes the normal gradient

\begin{equation}
\vec{n} \cdot D \nabla u = D q_N
\end{equation}

where $q_N$ is the user-supplied normal gradient through
the [!param](/BCs/DiffusionLHDGPrescribedGradientBC/normal_gradient)
parameter. This boundary condition should be paired with a
[DiffusionLHDGKernel.md].

!syntax parameters /BCs/DiffusionLHDGPrescribedGradientBC

!syntax inputs /BCs/DiffusionLHDGPrescribedGradientBC

!syntax children /BCs/DiffusionLHDGPrescribedGradientBC
