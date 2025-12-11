# MassContinuityIPHDGKernel

This class implements the following weak form on domain interior faces

\begin{equation}
\int_{F} \bar{q} \left(u - \bar{u}\right) \cdot \hat{n} dS
\end{equation}

where $\bar{q}$ are the test functions for the facet pressure, $u$ is the velocity
on element interiors, $\hat{u}$ is the facet velocity, and $\hat{n}$ is the face normal.
This form weakly enforces velocity continuity (mass continuity for incompressible formulations).

!syntax parameters /HDGKernels/MassContinuityIPHDGKernel

!syntax inputs /HDGKernels/MassContinuityIPHDGKernel

!syntax children /HDGKernels/MassContinuityIPHDGKernel
