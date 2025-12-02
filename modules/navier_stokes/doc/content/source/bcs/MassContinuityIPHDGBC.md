# MassContinuityIPHDGBC

This class implements the following weak form on boundary faces

\begin{equation}
\int_{\partial\Omega} \bar{q} \left(u - \bar{u}\right) \cdot \hat{n} dS
\end{equation}

where $\bar{q}$ are the test functions for the facet pressure, $u$ is the velocity
on element interiors, $\hat{u}$ is the facet velocity, and $\hat{n}$ is the boundary normal.
This form weakly enforces velocity continuity (mass continuity for incompressible formulations).

!syntax parameters /BCs/MassContinuityIPHDGBC

!syntax inputs /BCs/MassContinuityIPHDGBC

!syntax children /BCs/MassContinuityIPHDGBC
