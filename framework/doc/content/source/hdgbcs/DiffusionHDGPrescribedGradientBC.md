# DiffusionHDGPrescribedGradientBC

In a hybridized discontinuous Galerkin (HDG) discretization, this class
prescribes the normal gradient

\begin{equation}
\vec{n} \cdot D \nabla u = D q_N
\end{equation}

where $q_N$ is the user-supplied normal gradient through
the [!param](/HDGBCs/DiffusionHDGPrescribedGradientBC/normal_gradient)
parameter. This boundary condition should be paired with a
[DiffusionHDGKernel.md].

!syntax parameters /HDGBCs/DiffusionHDGPrescribedGradientBC

!syntax inputs /HDGBCs/DiffusionHDGPrescribedGradientBC

!syntax children /HDGBCs/DiffusionHDGPrescribedGradientBC
